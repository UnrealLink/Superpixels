#pragma once

#include "image.h"
#include <vector>

typedef struct Centroid{
    float L, a, b, x, y;
} Centroid;

class Slic{
    public:
        Slic(Image<Vec3b> _imageLab, int _k, float _m=10);
        ~Slic(){};

        void showSuperpixels();

    private:
        Image<Vec3b> imageLab;
        int k;
        float m;
        Image<int> superpixels;
        vector<Centroid> centroids;
};