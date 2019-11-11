#include "slic.h"
#include <math.h>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

vector<Centroid> initialize(const Image<Vec3b>& imageLab, int k){
    vector<Centroid> centroids(k);
    int w = imageLab.width();
    int h = imageLab.height();
    float S = sqrt(h*w/k); // roughly the distance between centroids
    int count = 0;
    int nb_w = (int)((float)(w) / S);
    int nb_h = (int)((float)(h) / S);
    int r = k - nb_w*nb_h;
    int y = S/2;
    for (int i=0; i<nb_h; i++){
        int nb_points;
        if (i < r){
            nb_points = (nb_w + 1);
        } else {
            nb_points = nb_w;
        }
        int step = w / nb_points;
        int x = step/2;
        for (int j=0; j<nb_points; j++){
            Centroid centroid;
            centroid.L = imageLab(x, y)[0];
            centroid.a = imageLab(x, y)[1];
            centroid.b = imageLab(x, y)[2];
            centroid.x = x;
            centroid.y =  y;
            centroids[count] = centroid;
            count++;
            x += step;
        }
        y += S;
    }
    return centroids;
}

Image<float> squareGradient(const Image<float>& imageGray) {
    int w = imageGray.width();
    int h = imageGray.height();
    Image<float> G(w, h);
    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            if (x == 0 || x == (w-1) || y == 0 || y == (h-1)) {
                G(x, y) = 0.;
            }
            else {
                float i_x = (imageGray(x+1, y) - imageGray(x-1, y)) * (imageGray(x+1, y) - imageGray(x-1, y)) / 4;
                float i_y = (imageGray(x, y+1) - imageGray(x, y-1)) * (imageGray(x, y+1) - imageGray(x, y-1)) / 4;
                G(x, y) = i_x + i_y;
            }
        }
    }
    return G;
}

void moveToSeeds(vector<Centroid>& centroids, const Image<Vec3b>& imageLab, int n = 3) {
    // compute square gradient and move centroids to seed locations with the lowest gradient in a nxn neighborhood
    int w = imageLab.width();
    int h = imageLab.height();
    Image<Vec3b> imageBGR(w, h);
    Image<uchar> imageGrayInter(w, h);
    Image<float> imageGray(w, h);
    cvtColor(imageLab, imageBGR, COLOR_Lab2BGR);
    cvtColor(imageBGR, imageGrayInter, COLOR_BGR2GRAY);
    imageGrayInter.convertTo(imageGray, CV_32F);

    Image<float> G = squareGradient(imageGray);

    int a = n / 2;
    for (int i = 0; i < centroids.size(); i++) {
        int x1 = max(0, (int)floor(centroids[i].x - a));
        int x2 = min(w, (int)floor(centroids[i].x + a + 1));
        int y1 = max(0, (int)floor(centroids[i].y - a));
        int y2 = min(h, (int)floor(centroids[i].y + a + 1));

        int x_min = x1;
        int y_min = y1;
        int grad_min = G(x1, y1);
        for (int x = x1; x < x2; x++) {
            for (int y = y1; y < y2; y++) {
                if (G(x, y) < grad_min) {
                    x_min = x; y_min = y;
                    grad_min = G(x, y);
                }
            }
        }
        centroids[i].x = x_min;
        centroids[i].y = y_min;
        centroids[i].L = imageLab(x_min, y_min)[0];
        centroids[i].a = imageLab(x_min, y_min)[1];
        centroids[i].b = imageLab(x_min, y_min)[2];
    }
}

void assignClusters(Image<uchar>& superpixels, const Image<Vec3b>& imageLab, const vector<Centroid>& centroids, float m) {
    // assign best matching pixels from a 2Sx2S neighborhood around each cluster center according to the distance measure
    int w = imageLab.width();
    int h = imageLab.height();
    float S = sqrt(w * h / centroids.size());

    for (int i = 0; i < centroids.size(); i++) {
        int label = i+1;
        int x1 = max(0, (int)floor(centroids[i].x - S));
        int x2 = min(w, (int)floor(centroids[i].x + S + 1));
        int y1 = max(0, (int)floor(centroids[i].y - S));
        int y2 = min(h, (int)floor(centroids[i].y + S + 1));

        for (int x = x1; x < x2; x++) {
            for (int y = y1; y < y2; y++) {
                if (superpixels(x, y) == 0) {
                    superpixels(x, y) = label;
                }
                else {
                    int j = superpixels(x, y) - 1;
                    float d_lab1 = sqrt((centroids[j].L - imageLab(x, y)[0])*(centroids[j].L - imageLab(x, y)[0])
                                    + (centroids[j].a - imageLab(x, y)[1])*(centroids[j].a - imageLab(x, y)[1])
                                    + (centroids[j].b - imageLab(x, y)[2])*(centroids[j].b - imageLab(x, y)[2]));
                    float d_xy1 = sqrt((centroids[j].x - x)*(centroids[j].x - x) + (centroids[j].y - y)*(centroids[j].y - y));
                    float Ds1 = d_lab1 + (m / S)*d_xy1;

                    float d_lab2 = sqrt((centroids[i].L - imageLab(x, y)[0])*(centroids[i].L - imageLab(x, y)[0])
                                    + (centroids[i].a - imageLab(x, y)[1])*(centroids[i].a - imageLab(x, y)[1])
                                    + (centroids[i].b - imageLab(x, y)[2])*(centroids[i].b - imageLab(x, y)[2]));
                    float d_xy2 = sqrt((centroids[i].x - x)*(centroids[i].x - x) + (centroids[i].y - y)*(centroids[i].y - y));
                    float Ds2 = d_lab2 + (m / S)*d_xy2;

                    if (Ds2 < Ds1) {
                        superpixels(x, y) = label;
                    }
                }
            }
        }
    }
}

float moveCentroids(const Image<uchar>& superpixels, const Image<Vec3b>& imageLab, vector<Centroid>& centroids) {
    float residual_error = 0.;

    // compute new cluster centers and residual error
    int w = imageLab.width();
    int h = imageLab.height();
    int *cluster_sizes = new int[centroids.size()];
    for (int i = 0; i < centroids.size(); i++) {
        cluster_sizes[i] = 0;
    }
    vector<Centroid> new_centroids(centroids.size());

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            if (superpixels(x, y) == 0) {
                continue;
            }

            int i = superpixels(x, y) - 1;
            cluster_sizes[i]++;
            new_centroids[i].x += x;
            new_centroids[i].y += y;
            new_centroids[i].L += imageLab(x, y)[0];
            new_centroids[i].a += imageLab(x, y)[1];
            new_centroids[i].b += imageLab(x, y)[2];
        }
    }

    for (int i = 0; i < centroids.size(); i++) {
        if (cluster_sizes[i] == 0) {
            new_centroids[i] = centroids[i];
        }
        else {
            new_centroids[i].x /= cluster_sizes[i];
            new_centroids[i].y /= cluster_sizes[i];
            new_centroids[i].L /= cluster_sizes[i];
            new_centroids[i].a /= cluster_sizes[i];
            new_centroids[i].b /= cluster_sizes[i];

            residual_error += abs(centroids[i].x - new_centroids[i].x);
            residual_error += abs(centroids[i].y - new_centroids[i].y);
            residual_error += abs(centroids[i].L - new_centroids[i].L);
            residual_error += abs(centroids[i].a - new_centroids[i].a);
            residual_error += abs(centroids[i].b - new_centroids[i].b);
        }
    }

    centroids = new_centroids;

    return residual_error;
}

void enforceConnectivity(Image<uchar>& superpixels, const vector<Centroid>& centroids) {
    // relabel disjoint segments to enforce connectivity of superpixels
}

Slic::Slic(Image<Vec3b> _imageLab, int _k, float _m){
    imageLab = _imageLab;
    k = _k;
    m = _m;
    centroids = initialize(imageLab, k);
    superpixels = Image<uchar>(imageLab.width(), imageLab.height());
    superpixels.setTo(0);
    // for (int i=0; i<k; i++){
    //     std::cout << centroids[i].x << " " << centroids[i].y << std::endl;
    // }
    moveToSeeds(centroids, imageLab);
    int max_iter = 10;
    float residual_error;
    for (int i = 0; i < max_iter; i++) {
        assignClusters(superpixels, imageLab, centroids, m);
        residual_error = moveCentroids(superpixels, imageLab, centroids);
        std::cout << "Residual error at step " << i << " : " << residual_error << std::endl;
    }
    enforceConnectivity(superpixels, centroids);
}

void Slic::showSuperpixels() {
    int w = imageLab.width();
    int h = imageLab.height();
    Image<Vec3b> I(w, h);

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            if (superpixels(x, y) == 0) {
                I(x, y)[0] = 0;
                I(x, y)[1] = 0;
                I(x, y)[2] = 0;
            }
            else {
                int i = superpixels(x, y) - 1;
                I(x, y)[0] = centroids[i].L;
                I(x, y)[1] = centroids[i].a;
                I(x, y)[2] = centroids[i].b;
            }
        }
    }

    cvtColor(I, I, COLOR_Lab2BGR);
    imshow("superpixels", I);
}
