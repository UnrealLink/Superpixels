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
        vector<int> getANNs(){ return ANNs; }
        vector<vector<int> >& getNeighbours(){ return neighbours1; };
        void showMatchs();

    private:
        Slic& superpixels1;
        Slic& superpixels2;
        int degree;
        int maxIter = 10;

        vector<int> ANNs;
        vector<list<int> > reverseANNs;
        vector<int> countMatchings;
        vector<int> orderedCentroids1;
        vector<vector<int> > neighbours1;
        vector<vector<float> > neighboursAngle1;
        vector<int> orderedCentroids2;
        vector<vector<int> > neighbours2;
        vector<vector<float> > neighboursAngle2;

        void initRandomANNs();
        void computeOrderAndNeighbours(Slic superpixels, vector<int>& orderedCentroids,
                                       vector<vector<int> >& neighbours,
                                       vector<vector<float> >& neighboursAngle);
        void propagate();
        void randomSearch();
        int selectRandomSuperpixel(int d, int x, int y);

};
