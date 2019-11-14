#pragma once

#include "image.h"
#include "slic.h"
#include <vector>
#include <list>

class SuperPatchMatcher{
    public:
        SuperPatchMatcher(Slic& _superpixels1, Slic& _superpixels2, int _degree=3);
        ~SuperPatchMatcher(){};

        void computeANNs();
        vector<int> getANNs();

    private:
        Slic& superpixels1;
        Slic& superpixels2;
        int degree;

        vector<int> ANNs;
        vector<int> orderedCentroids;
        vector<vector<int> > neighbours;
        vector<vector<float> > neighboursAngle;

        void initRandomANNs();
        void computeOrderAndNeighbours();
        void propagate();
        void randomSearch();

};