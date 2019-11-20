#include "fusion.h"

#include <vector>
#include <iostream>
#include <math.h>
#include <iterator>
#include <algorithm>

using namespace cv;

/*
    Spatial and colorimetric covariance of superpixels in image A.
*/

void computeSpatialCov(vector<vector<float> > &spatial_cov, const Slic &slic1, float delta_s) {
    int w = slic1.getImage().width();
    int h = slic1.getImage().height();
    int k = slic1.getNbSuperpixels();

    vector<int> cluster_sizes(k, 0);
    vector<float> sum_x(k, 0.);
    vector<float> sum_y(k, 0.);
    vector<float> sqr_x(k, 0.);
    vector<float> sqr_y(k, 0.);

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            int i = slic1.getSuperpixels()(x, y);
            cluster_sizes[i]++;

            float x_norm = (float)x / (float)w;
            sum_x[i] += x_norm;
            sqr_x[i] += x_norm * x_norm;

            float y_norm = (float)y / (float)w;
            sum_y[i] += y_norm;
            sqr_y[i] += y_norm * y_norm;
        }
    }

    for (int i = 0; i < k; i++) {
        float mean1_x = sum_x[i] / cluster_sizes[i];
        float mean2_x = sqr_x[i] / cluster_sizes[i];
        spatial_cov[i][0] = (mean2_x - (mean1_x * mean1_x)) * delta_s * delta_s;

        float mean1_y = sum_y[i] / cluster_sizes[i];
        float mean2_y = sqr_y[i] / cluster_sizes[i];
        spatial_cov[i][1] = (mean2_y - (mean1_y * mean1_y)) * delta_s * delta_s;
    }
}

void computeColorCov(vector<vector<float> > &color_cov, const Slic &slic1, float delta_c) {
    int w = slic1.getImage().width();
    int h = slic1.getImage().height();
    int k = slic1.getNbSuperpixels();

    vector<int> cluster_sizes(k, 0);
    vector<float> sum_L(k, 0.);
    vector<float> sum_a(k, 0.);
    vector<float> sum_b(k, 0.);
    vector<float> sqr_L(k, 0.);
    vector<float> sqr_a(k, 0.);
    vector<float> sqr_b(k, 0.);

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            int i = slic1.getSuperpixels()(x, y);
            cluster_sizes[i]++;

            float L_norm = slic1.getCentroids()[i].L / 255;
            sum_L[i] += L_norm;
            sqr_L[i] += L_norm * L_norm;

            float a_norm = slic1.getCentroids()[i].a / 255;
            sum_a[i] += a_norm;
            sqr_a[i] += a_norm * a_norm;

            float b_norm = slic1.getCentroids()[i].b / 255;
            sum_b[i] += b_norm;
            sqr_b[i] += b_norm * b_norm;
        }
    }

    for (int i = 0; i < k; i++) {
        float mean1_L = sum_L[i] / cluster_sizes[i];
        float mean2_L = sqr_L[i] / cluster_sizes[i];
        color_cov[i][0] = (mean2_L - (mean1_L * mean1_L)) * delta_c * delta_c;

        float mean1_a = sum_a[i] / cluster_sizes[i];
        float mean2_a = sqr_a[i] / cluster_sizes[i];
        color_cov[i][1] = (mean2_a - (mean1_a * mean1_a)) * delta_c * delta_c;

        float mean1_b = sum_b[i] / cluster_sizes[i];
        float mean2_b = sqr_b[i] / cluster_sizes[i];
        color_cov[i][2] = (mean2_b - (mean1_b * mean1_b)) * delta_c * delta_c;
    }
}

/*
    Compute color transfer image pixel per pixel.
*/

void transferColor(Image<Vec3b> &transferImage,
                   const Slic &slic1, const Slic &slic2, const SuperPatchMatcher &spm,
                   const vector<vector<float> > &spatial_cov, const vector<vector<float> > &color_cov) {
    int w = transferImage.width();
    int h = transferImage.height();
    int k = slic1.getNbSuperpixels();

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            // compute distances weighted with covariance
            vector<float> distances(k, 0.);
            for (int i = 0; i < k; i++) {
                distances[i] += ((x - slic1.getCentroids()[i].x)/w)*((x - slic1.getCentroids()[i].x)/w) / spatial_cov[i][0];
                distances[i] += ((y - slic1.getCentroids()[i].y)/h)*((y - slic1.getCentroids()[i].y)/h) / spatial_cov[i][1];

                distances[i] += ((slic1.getImage()(x, y)[0] - slic1.getCentroids()[i].L)/255)*((slic1.getImage()(x, y)[0] - slic1.getCentroids()[i].L)/255) / color_cov[i][0];
                distances[i] += ((slic1.getImage()(x, y)[1] - slic1.getCentroids()[i].a)/255)*((slic1.getImage()(x, y)[1] - slic1.getCentroids()[i].a)/255) / color_cov[i][1];
                distances[i] += ((slic1.getImage()(x, y)[2] - slic1.getCentroids()[i].b)/255)*((slic1.getImage()(x, y)[2] - slic1.getCentroids()[i].b)/255) / color_cov[i][2];
            }

            // compute weights
            float sigma = std::min_element(std::begin(distances), std::end(distances));
            vector<float> weights(k);
            for (int i = 0; i < k; i++) {
                weights[i] = std::exp((-1) * (distances[i] - sigma));
            }

            // compute transfered color
            float total_weight = 0.;
            float transfer_L = 0.;
            float transfer_a = 0.;
            float transfer_b = 0.;
            for (int i = 0; i < k; i++) {
                total_weight += weights[i];
                transfer_L += weights[i] * slic2.getCentroids()[spm.getANNs()[i]].L / 255;
                transfer_a += weights[i] * slic2.getCentroids()[spm.getANNs()[i]].a / 255;
                transfer_b += weights[i] * slic2.getCentroids()[spm.getANNs()[i]].b / 255;
            }
            transfer_L /= total_weight;
            transfer_a /= total_weight;
            transfer_b /= total_weight;

            transferImage(x, y)[0] = 255 * transfer_L;
            transferImage(x, y)[1] = 255 * transfer_a;
            transferImage(x, y)[2] = 255 * transfer_b;
        }
    }
}

/*
    Implementation of fusion methods.
*/

ColorFusion::ColorFusion(const Slic &_slic1, const Slic &_slic2, const SuperPatchMatcher &_spm, float _delta_c, float _delta_s){
    slic1 = _slic1;
    slic2 = _slic2;
    spm = _spm;
    delta_c = _delta_c;
    delta_s = _delta_s;

    int w = slic1.getImage().width();
    int h = slic1.getImage().height();
    int k = slic1.getNbSuperpixels();

    vector<vector<float> > spatial_cov(k);
    vector<vector<float> > color_cov(k);
    for (int i = 0; i < k; i++) {
        spatial_cov[i] = vector<float>(2);
        color_cov[i] = vector<float>(3);
    }
    computeSpatialCov(spatial_cov, slic1, delta_s);
    computeColorCov(color_cov, slic1, delta_c);

    transferImage = Image<Vec3b>(w, h);
    transferColor(transferImage, slic1, slic2, spm, spatial_cov, color_cov);
}

void ColorFusion::showTransferImage() {
    int w = transferImage.width();
    int h = transferImage.height();
    Image<Vec3b> I(w, h);

    cvtColor(transferImage, I, COLOR_Lab2BGR);
    imshow("color transfer", I);
}
