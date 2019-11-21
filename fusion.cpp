#include "fusion.h"

#include <iostream>
#include <math.h>
#include <iterator>
#include <algorithm>

using namespace std;
using namespace cv;

/*
    Initialization and computation of covariance matrices, weights and transfered colors.
*/

ColorFusion::ColorFusion(Slic &_slic1, Slic &_slic2, SuperPatchMatcher &_spm, float _delta_c, float _delta_s) :
    slic1(_slic1), slic2(_slic2), spm(_spm)
{
    cout << "Entering color fusion..." << endl;
    delta_c = _delta_c;
    delta_s = _delta_s;

    int w = slic1.getImage().width();
    int h = slic1.getImage().height();
    int k = slic1.getNbSuperpixels();

    spatial_cov = vector<Matx22f>(k);
    color_cov = vector<Matx33f>(k);
    for (int i = 0; i < k; i++) {
        spatial_cov[i] = Matx22f::zeros();
        color_cov[i] = Matx33f::zeros();
    }
    computeCovs();
    cout << "Covariances : done" << endl;

    transferImage = Image<Vec3b>(w, h);
    computeTransferImage();
    cout << "Transfer image : done" << endl;
}

/*
    Spatial and colorimetric covariance of superpixels in image A.
*/

void ColorFusion::computeCovs() {
    int w = slic1.getImage().width();
    int h = slic1.getImage().height();
    int k = slic1.getNbSuperpixels();

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            int i = slic1.getSuperpixels()(x, y);

            // center and normalize 5D vector correponding to current pixel
            float x_norm = ((float)x - slic1.getCentroids()[i].x) / w;
            float y_norm = ((float)y - slic1.getCentroids()[i].y) / h;
            float L_norm = ((float)slic1.getImage()(x, y)[0] - slic1.getCentroids()[i].L) / 255;
            float a_norm = ((float)slic1.getImage()(x, y)[1] - slic1.getCentroids()[i].a) / 255;
            float b_norm = ((float)slic1.getImage()(x, y)[2] - slic1.getCentroids()[i].b) / 255;
            Vec2f pos(x_norm, y_norm);
            pos *= delta_s;
            Vec3f color(L_norm, a_norm, b_norm);
            color *= delta_c;

            // update spatial covariance of superpixel
            for (int r = 0; r < 2; r++) {
                for (int c = 0; c < 2; c++) {
                    spatial_cov[i](r, c) += pos[r]*pos[c];
                }
            }

            // update color covariance of superpixel
            for (int r = 0; r < 3; r++) {
                for (int c = 0; c < 3; c++) {
                    color_cov[i](r, c) += color[r]*color[c];
                }
            }
        }
    }

    // relative importance factor between spatial and color distribution
    for (int i = 0; i < k; i++) {
        spatial_cov[i] = spatial_cov[i].inv();
        color_cov[i] = color_cov[i].inv();
    }
}

/*
    Color transfer.
    For each pixel, compute k weights to take into account the distance to each superpixel centroid.
*/

void ColorFusion::computeTransferImage() {
    int w = transferImage.width();
    int h = transferImage.height();
    int k = slic1.getNbSuperpixels();

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            vector<float> distances(k, 0.);
            for (int i = 0; i < k; i++) {
                // compute 5D vector corresponding to the difference between current pixel and current superpixel
                float x_diff = ((float)x - slic1.getCentroids()[i].x) / w;
                float y_diff = ((float)y - slic1.getCentroids()[i].y) / h;
                float L_diff = ((float)slic1.getImage()(x, y)[0] - slic1.getCentroids()[i].L) / 255;
                float a_diff = ((float)slic1.getImage()(x, y)[1] - slic1.getCentroids()[i].a) / 255;
                float b_diff = ((float)slic1.getImage()(x, y)[2] - slic1.getCentroids()[i].b) / 255;
                Vec2f pos(x_diff, y_diff);
                Vec3f color(L_diff, a_diff, b_diff);

                // compute distance in xylab space with respect to covariance
                distances[i] += pos.dot(spatial_cov[i] * pos);
                distances[i] += color.dot(color_cov[i] * color);
            }

            // compute weights
            float sigma = *min_element(begin(distances), end(distances));
            vector<float> weights(k);
            float total_weights = 0.;
            for (int i = 0; i < k; i++) {
                weights[i] = exp((-1) * (distances[i] - sigma));
                total_weights += weights[i];
            }

            // color transfer
            Vec3f new_color(0., 0., 0.);
            for (int i = 0; i < k; i++) {
                Vec3f cur_color(slic2.getCentroids()[spm.getANNs()[i]].L, slic2.getCentroids()[spm.getANNs()[i]].a, slic2.getCentroids()[spm.getANNs()[i]].b);
                new_color += weights[i]*cur_color;
            }
            new_color /= total_weights;
            if (x % 50 == 0 && y % 50 == 0) { cout << new_color << endl; }
            for (int j = 0; j < 3; j++) {
                transferImage(x, y)[j] = (int)new_color[j];
            }
        }
    }
}

/*
    Show image after color transfer.
*/

void ColorFusion::showTransferImage() {
    int w = transferImage.width();
    int h = transferImage.height();
    Image<Vec3b> I(w, h);

    cvtColor(transferImage, I, COLOR_Lab2BGR);
    imshow("color transfer", I);
}
