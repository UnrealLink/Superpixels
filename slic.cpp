#include "slic.h"
#include <math.h>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <queue>

using namespace std;
using namespace cv;

/*
    Initialize centroids by placing them in a roughly uniform manner in the image,
    then move them to the point in their 3x3 neighborhood where the squared gradient is the lowest.
*/

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

/*
    At each iteration of the algorithm, update the centroids positions
    and compute the Voronoï partition of the image with respect to the centroids in the Labxy space.
*/

float cieLabDist(Centroid centroid1, Centroid centroid2){
    float d_lab = sqrt((centroid1.L - centroid2.L)*(centroid1.L - centroid2.L)
                        + (centroid1.a - centroid2.a)*(centroid1.a - centroid2.a)
                        + (centroid1.b - centroid2.b)*(centroid1.b - centroid2.b));
    return d_lab;
}

float cieLabDist(Point p, const Image<Vec3b>& imageLab, int i, const vector<Centroid>& centroids, float S, float m) {
    float d_lab = sqrt((centroids[i].L - imageLab(p)[0])*(centroids[i].L - imageLab(p)[0])
                        + (centroids[i].a - imageLab(p)[1])*(centroids[i].a - imageLab(p)[1])
                        + (centroids[i].b - imageLab(p)[2])*(centroids[i].b - imageLab(p)[2]));
    float d_xy = sqrt((centroids[i].x - p.x)*(centroids[i].x - p.x) + (centroids[i].y - p.y)*(centroids[i].y - p.y));
    float Ds = d_lab + (m / S)*d_xy;
    return Ds;
}

void assignClusters(Image<int>& superpixels, const Image<Vec3b>& imageLab, const vector<Centroid>& centroids, float m) {
    // assign best matching pixels from a 2Sx2S neighborhood around each cluster center according to the distance measure
    int w = imageLab.width();
    int h = imageLab.height();
    int k = centroids.size();
    float S = sqrt(w * h / k);

    for (int i = 0; i < k; i++) {
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
                    float Ds1 = cieLabDist(Point(x, y), imageLab, j, centroids, S, m);
                    float Ds2 = cieLabDist(Point(x, y), imageLab, i, centroids, S, m);
                    if (Ds2 < Ds1) {
                        superpixels(x, y) = label;
                    }
                }
            }
        }
    }
}

float moveCentroids(const Image<int>& superpixels, const Image<Vec3b>& imageLab, vector<Centroid>& centroids) {
    float residual_error = 0.;

    // compute new cluster centers and residual error
    int w = imageLab.width();
    int h = imageLab.height();
    int k = centroids.size();

    int *cluster_sizes = new int[centroids.size()];
    for (int i = 0; i < centroids.size(); i++) {
        cluster_sizes[i] = 0;
    }
    vector<Centroid> new_centroids(centroids.size());

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            if (superpixels(x, y) == 0) continue;

            int i = superpixels(x, y) - 1;
            cluster_sizes[i]++;
            new_centroids[i].x += x;
            new_centroids[i].y += y;
            new_centroids[i].L += imageLab(x, y)[0];
            new_centroids[i].a += imageLab(x, y)[1];
            new_centroids[i].b += imageLab(x, y)[2];
        }
    }

    for (int i = 0; i < k; i++) {
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

/*
    A few pixels may remain unconnected to the superpixels,
    here we explicitely enforce connectivity.
*/

void enforceConnectivity(Image<int>& superpixels, const vector<Centroid>& centroids, const Image<Vec3b>& imageLab, float m) {
    // relabel disjoint segments to enforce connectivity of superpixels

    // go through image to get, for each centroid, the whole cluster and its seed (the point closer to the centroid)
    int w = superpixels.width();
    int h = superpixels.height();
    int k = centroids.size();
    float S = sqrt(w * h / k);

    vector<Point> seeds(k);
    for (int i = 0; i < k; i++) {
        seeds[i] = Point(0, 0);
    }
    vector<queue<Point> > clusters(k+1);

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            // add point at coordinates (x, y) to corresponding cluster
            clusters[superpixels(x, y)].push(Point(x, y));
            if (superpixels(x, y) == 0) continue;

            // check if it is closer to the centroid than the current seed of the cluster
            int j = superpixels(x, y) - 1;
            float Ds1 = cieLabDist(Point(x, y), imageLab, j, centroids, S, m);
            float Ds2 = cieLabDist(seeds[j], imageLab, j, centroids, S, m);
            if (Ds1 < Ds2) {
                seeds[j] = Point(x, y);
            }
        }
    }

    // perform BFS on each cluster, starting at its seed
    Image<uchar> seen(w, h);
    seen.setTo(0);
    for (int i = 1; i < k+1; i++) {
        queue<Point> Q;
        Q.push(seeds[i-1]);
        int x = seeds[i-1].x; int y = seeds[i-1].y;
        seen(x, y) = 1;
        while (!Q.empty()) {
            int x = Q.front().x; int y = Q.front().y;
            Q.pop();
            if (x > 0 && superpixels(x-1, y) == i && !seen(x-1, y)) {
                seen(x-1, y) = 1;
                Q.push(Point(x-1, y));
            }
            if (x < (w-1) && superpixels(x+1, y) == i && !seen(x+1, y)) {
                seen(x+1, y) = 1;
                Q.push(Point(x+1, y));
            }
            if (y > 0 && superpixels(x, y-1) == i && !seen(x, y-1)) {
                seen(x, y-1) = 1;
                Q.push(Point(x, y-1));
            }
            if (y < (h-1) && superpixels(x, y+1) == i && !seen(x, y+1)) {
                seen(x, y+1) = 1;
                Q.push(Point(x, y+1));
            }
        }
    }
    imshow("seen", 255 * seen);

    // relabel remaining disjoint segments
    int n_remaining = k+1;
    while (n_remaining > 0) {
        for (int i = 0; i < k+1; i++) {
            queue<Point> residual;
            while (!clusters[i].empty()) {
                int x = clusters[i].front().x; int y = clusters[i].front().y;
                clusters[i].pop();
                if (seen(x, y)) continue;

                queue<Point> neighbours;
                if (x > 0 && seen(x-1, y)) {
                    neighbours.push(Point(x-1, y));
                }
                if (x < (w-1) && seen(x+1, y)) {
                    neighbours.push(Point(x+1, y));
                }
                if (y > 0 && seen(x, y-1)) {
                    neighbours.push(Point(x, y-1));
                }
                if (y < (h-1) && seen(x, y+1)) {
                    neighbours.push(Point(x, y+1));
                }

                if (neighbours.empty()) {
                    residual.push(Point(x, y));
                }
                else {
                    int new_label = superpixels(neighbours.front());
                    int j = new_label - 1;
                    float min_dist = cieLabDist(neighbours.front(), imageLab, j, centroids, S, m);
                    neighbours.pop();
                    while (!neighbours.empty()) {
                        int cur_label = superpixels(neighbours.front());
                        int j = cur_label - 1;
                        float cur_dist = cieLabDist(neighbours.front(), imageLab, j, centroids, S, m);
                        new_label = (cur_dist < min_dist) ? cur_label : new_label;
                        neighbours.pop();
                    }
                    superpixels(x, y) = new_label;
                    seen(x, y) = 1;
                }
            }
            clusters[i] = residual;
            if (clusters[i].empty()) n_remaining--;
        }
    }
}

/*
    Implementation of Slic methods.
*/

Slic::Slic(Image<Vec3b> _imageLab, int _k, float _m){
    imageLab = _imageLab;
    k = _k;
    m = _m;

    centroids = initialize(imageLab, k);
    moveToSeeds(centroids, imageLab);

    superpixels = Image<int>(imageLab.width(), imageLab.height());
    superpixels.setTo(0);
    assignClusters(superpixels, imageLab, centroids, m);

    int max_iter = 10;
    float residual_error;
    for (int i = 1; i <= max_iter; i++) {
        residual_error = moveCentroids(superpixels, imageLab, centroids);
        std::cout << "Residual error at step " << i << " : " << residual_error << std::endl;
        assignClusters(superpixels, imageLab, centroids, m);
    }

    enforceConnectivity(superpixels, centroids, imageLab, m);
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
