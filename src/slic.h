#pragma once

#include "image.h"
#include <vector>

typedef struct Centroid{
    float L, a, b, x, y;
} Centroid;

float cieLabDist(Centroid centroid1, Centroid centroid2);
float cieLabDist(Point p, const Image<Vec3b>& imageLab, int i, const vector<Centroid>& centroids, float S, float m);

class Slic{
    public:
        Slic(Image<Vec3b> _imageLab, int _k, float _m=10);
        ~Slic(){};

        void showSuperpixels(string title);

        const Image<Vec3b>& getImage(){return imageLab;}
        const Image<int>& getSuperpixels(){return superpixels;}
	    int getCluster(int x, int y){return superpixels(x, y);}
        int getNbSuperpixels(){return k;}
        const vector<Centroid>& getCentroids(){return centroids;}
	    Centroid getCentroid(int i){return centroids[i];}

    private:
        Image<Vec3b> imageLab;
        int k;
        float m;
        Image<int> superpixels;
        vector<Centroid> centroids;
};
