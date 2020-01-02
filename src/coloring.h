#pragma once

#include "image.h"
#include "slic.h"

#include <opencv2/highgui/highgui.hpp>
#include <Eigen/Core>
#include <Eigen/Sparse>
#include <Eigen/SVD>
#include <Eigen/Eigenvalues>
#include <vector>
#include <algorithm>

using namespace Eigen;

class Coloring{
    public:
        Coloring(Slic& _superpixels, vector<int> _filledClusters, vector<Vec3b> _colors);
        ~Coloring();

        const vector<Vec3b>& getNewColors(){return newColors;}
        vector<vector<int> >& getNeighbours(){return neighbours;}

    private:
        Slic& superpixels;
        vector<int> clustersY;
        vector<int> filledClusters;
        vector<Vec3b> colors;
        vector<vector<int> > neighbours;
        SparseMatrix<float> affinityMatrix;
        vector<Vec3b> newColors;

        void convertClustersToYUV();
        void computeNeighbours(vector<vector<int> >& neighbours);
        void computeAffinityMatrix(SparseMatrix<float>& affinityMatrix);
        void solveAffinityMatrix();
        void convertClustersToLab();
};

float computeMean(const vector<float>& array);
float computeVar(const vector<float>& array, float mean);