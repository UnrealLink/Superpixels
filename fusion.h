#pragma once

#include "image.h"
#include "slic.h"
#include "spm.h"

#include <opencv2/highgui/highgui.hpp>
#include <vector>

class ColorFusion{
    public:
        ColorFusion(Slic &_slic1, Slic &_slic2, SuperPatchMatcher &_spm, float _delta_c = 0.1, float _delta_s = 10.);
        ~ColorFusion(){};

        void showTransferImage();

    private:
        Slic& slic1;
        Slic& slic2;
        SuperPatchMatcher& spm;
        float delta_c;
        float delta_s;

        vector<Matx22f> spatial_cov;
        vector<Matx33f> color_cov;
        Image<Vec3b> transferImage;

        void computeCovs();
        vector<int> getNeighborhood(int i, vector<vector<int> > &neighbours, int radius = 3);
        void computeTransferImage();
};
