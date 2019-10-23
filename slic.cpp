#include "slic.h"
#include <math.h>
#include <iostream>

using namespace std;

vector<Centroid> initialize(Image<Vec3b> imageLab, int k){
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

Slic::Slic(Image<Vec3b> _imageLab, int _k, float _m){
    imageLab = _imageLab;
    k = _k;
    m = _m;
    centroids = initialize(imageLab, k);
    // for (int i=0; i<k; i++){
    //     std::cout << centroids[i].x << " " << centroids[i].y << std::endl;
    // }
    
}