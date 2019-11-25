#include "fusion.h"

#include <iostream>
#include <math.h>
#include <iterator>
#include <algorithm>
#include <queue>

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
    For each pixel, compute kn weights to take into account the distance to the neighbouring superpixels.
*/

vector<int> ColorFusion::getNeighborhood(int i, vector<vector<int> > &neighbours, int radius) {
    vector<int> neighborhood;

    int k = slic1.getNbSuperpixels();
    vector<bool> seen(k, false);
    vector<int> depth(k);
    queue<int> Q;

    Q.push(i);
    seen[i] = true;
    depth[i] = 0;
    while (!Q.empty()) {
        int s = Q.front();
        Q.pop();
        neighborhood.push_back(s);
        // s is on the border of the ball (in the graph space)
        if (depth[s] == radius) continue;
        // s is in the open ball
        for (vector<int>::iterator it = neighbours[s].begin(); it != neighbours[s].end(); it++) {
            if (seen[*it]) continue;
            seen[*it] = true;
            depth[*it] = depth[s] + 1;
            Q.push(*it);
        }
    }

    return neighborhood;
}

void ColorFusion::computeNeighbours(int radius) {
    int k = slic1.getNbSuperpixels();
    neighbours = vector<vector<int> >(k);
    for (int i = 0; i < k; i++) {
	neighbours[i] = getNeighborhood(i, spm.getNeighbours(), radius);
    }
}

void ColorFusion::computeTransferImage() {
    int w = transferImage.width();
    int h = transferImage.height();
    computeNeighbours();

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            // get superpixel's neighborhood
            int i = slic1.getSuperpixels()(x, y);
            vector<int> neighborhood = neighbours[i];
            int kn = neighborhood.size();

            vector<float> distances(kn, 0.);
            for (int j = 0; j < kn; j++) {
                // compute 5D vector corresponding to the difference between current pixel and current superpixel
                float x_diff = ((float)x - slic1.getCentroids()[neighborhood[j]].x) / w;
                float y_diff = ((float)y - slic1.getCentroids()[neighborhood[j]].y) / h;
                float L_diff = ((float)slic1.getImage()(x, y)[0] - slic1.getCentroids()[neighborhood[j]].L) / 255;
                float a_diff = ((float)slic1.getImage()(x, y)[1] - slic1.getCentroids()[neighborhood[j]].a) / 255;
                float b_diff = ((float)slic1.getImage()(x, y)[2] - slic1.getCentroids()[neighborhood[j]].b) / 255;
                Vec2f pos(x_diff, y_diff);
                Vec3f color(L_diff, a_diff, b_diff);

                // compute distance in xylab space with respect to covariance
                distances[j] += pos.dot(spatial_cov[i] * pos);
                distances[j] += color.dot(color_cov[i] * color);
            }

            // compute weights
            float sigma = *min_element(distances.begin(), distances.end());
            vector<float> weights(kn);
            float total_weights = 0.;
            for (int j = 0; j < kn; j++) {
                weights[j] = exp((-1) * (distances[j] - sigma));
                total_weights += weights[j];
            }

            // color transfer
            Vec3f new_color(0., 0., 0.);
            for (int j = 0; j < kn; j++) {
                Vec3f cur_color(slic2.getCentroids()[spm.getANNs()[neighborhood[j]]].L,
                                slic2.getCentroids()[spm.getANNs()[neighborhood[j]]].a,
                                slic2.getCentroids()[spm.getANNs()[neighborhood[j]]].b);
                new_color += weights[j]*cur_color;
            }
            new_color /= total_weights;

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
