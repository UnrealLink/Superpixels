#pragma once

#include "image.h"
#include "slic.h"
#include "spm.h"
#include "coloring.h"

#include <opencv2/highgui/highgui.hpp>
#include <vector>

class ColorFusion{
    public:
        ColorFusion(Slic *_slic1, Slic *_slic2, SuperPatchMatcher *_spm, float _delta_c = 0.1, float _delta_s = 10.);
        ColorFusion(Slic *_slic, Coloring* _coloring, float _delta_c = 0.1, float _delta_s = 10.);
        ~ColorFusion(){};

        void showTransferImage();

    private:
        Slic* slic1;
        Slic* slic2;
        SuperPatchMatcher* spm;
        Coloring* coloring;
        vector<Vec3b> matchedColors;
        float delta_c;
        float delta_s;

        vector<Matx22f> spatial_cov;
        vector<Matx33f> color_cov;
        Image<Vec3b> transferImage;

	vector<vector<int> > neighbours;

        void computeCovs();
        vector<int> getNeighborhood(int i, vector<vector<int> > &neighbours, int radius);
	void computeNeighboursFromSpm(int radius = 3);
        void computeNeighboursFromColoring(int radius = 3);
        void computeMatchedColorsFromSpm();
        void computeMatchedColorsFromColoring();
        void computeTransferImage();
};
