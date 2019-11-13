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

        const Image<Vec3b>& getImage(){return imageLab;}
        const Image<int>& getSuperpixels(){return superpixels;}
        int getNbSuperpixels(){return k;}
        const vector<Centroid>& getCentroids(){return centroids;}

    private:
        Image<Vec3b> imageLab;
        int k;
        float m;
        Image<int> superpixels;
        vector<Centroid> centroids;
};
