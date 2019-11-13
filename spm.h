#pragma once

#include "image.h"
#include "slic.h"
#include <vector>

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

        void initRandomANNs();

};