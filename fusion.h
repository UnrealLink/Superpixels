#pragma once

#include "image.h"
#incude "slic.h"
#include "spm.h"

class ColorFusion{
    public:
        ColorFusion(const Slic &_slic1, const Slic &_slic2, const SuperPatchMatcher &_spm, float _delta_c = 0.1, float _delta_s = 10.);
        ~ColorFusion(){};

        void showTransferImage();

    private:
        Slic& slic1;
        Slic& slic2;
        SuperPatchMatcher& spm;
        float delta_c;
        float delta_s;

        Image<Vec3b> transferImage;
};
