#include "spm.h"
#include <iostream>
#include <stdlib.h>
#include <cmath>

SuperPatchMatcher::SuperPatchMatcher(Slic& _superpixels1, Slic& _superpixels2, int _degree) : 
    superpixels1(_superpixels1), superpixels2(_superpixels2) 
{
    degree = _degree;
    ANNs = vector<int>(superpixels1.getNbSuperpixels(), 0);
}

void SuperPatchMatcher::computeANNs(){
    initRandomANNs();
    computeOrderAndNeighbours(superpixels1, orderedCentroids1, neighbours1, neighboursAngle1);
    computeOrderAndNeighbours(superpixels2, orderedCentroids2, neighbours2, neighboursAngle2);
    for (int i=0; i < maxIter; i++){
        propagate();
        randomSearch();
    }
}

void SuperPatchMatcher::initRandomANNs(){
    int n = superpixels1.getNbSuperpixels();
    int m = superpixels2.getNbSuperpixels();
    if (m*degree < n) throw std::invalid_argument( "not enough superpixels in image 2 with current degree" );
    countMatchings = vector<int>(m, 0); // Max assignment of a single superpixel as an ANN has to be less than degree
    srand (0); // initialize random seed
    for (int i=0; i<n; i++){
        ANNs[i] = rand() % m;
        while (countMatchings[ANNs[i]] >= degree){
           ANNs[i] = rand() % m; 
        }
        countMatchings[ANNs[i]]++;
    }
}

float computeAngle(Point p1, Point p2){
    if (p1.x == p2.x) {
        if (p1.y == p2.y) return 0;
        return (p1.y - p2.y)/abs(p1.y - p2.y) * atan(1)*2;
    }
    else if (p1.x > p2.x) {
        if (p1.y == p2.y) {
            return atan(1)*4;
        }
        else if (p1.y > p2.y) {
            return atan(abs(p1.x - p2.x)/abs(p1.y - p2.y)) + atan(1)*2;
        }
        else {
            return - atan(abs(p1.x - p2.x)/abs(p1.y - p2.y)) - atan(1)*2;
        }
    }
    else {
        if (p1.y == p2.y) return 0;
        return atan((p1.x - p2.x)/(p1.y - p2.y));
    }
}

void SuperPatchMatcher::computeOrderAndNeighbours(Slic superpixels, vector<int>& orderedCentroids, 
                                                  vector<vector<int> >& neighbours, vector<vector<float> >& neighboursAngle){
    int w = superpixels.getImage().width();
    int h = superpixels.getImage().height();
    int nbSuperpixels = superpixels.getNbSuperpixels();
    vector<int> _orderedCentroids(nbSuperpixels, -1); // inverted vector of orders (label -> order)
    vector<vector<bool> > _neighbours(nbSuperpixels); // flag vector of neighbours
    for (int i=0; i < nbSuperpixels; i++){
        _neighbours[i] = vector<bool>(nbSuperpixels, false);
    }
    int k=0;
    for (int x=0; x < w; x++){
        for (int y=0; y < h; y++){
            // update order
            int label = superpixels.getSuperpixels()(x, y);
            if (_orderedCentroids[label] == -1) {
                _orderedCentroids[label] = k;
                k++;
            }
            // Check neighbours
            if (x > 0 && !_neighbours[label][superpixels.getSuperpixels()(x-1, y)]) {
                _neighbours[label][superpixels.getSuperpixels()(x-1, y)] = true;
            }
            if (x < (w-1) && !_neighbours[label][superpixels.getSuperpixels()(x+1, y)]) {
                _neighbours[label][superpixels.getSuperpixels()(x+1, y)] = true;
            }
            if (y > 0 && !_neighbours[label][superpixels.getSuperpixels()(x, y-1)]) {
                _neighbours[label][superpixels.getSuperpixels()(x, y-1)] = true;
            }
            if (y < (h-1) && !_neighbours[label][superpixels.getSuperpixels()(x, y+1)]) {
                _neighbours[label][superpixels.getSuperpixels()(x, y+1)] = true;
            }
        }
    }

    // invert order vector (order -> label)
    orderedCentroids = vector<int>(nbSuperpixels, 0);
    for (int i=0; i < nbSuperpixels; i++){
        orderedCentroids[i] = std::distance(_orderedCentroids.begin(), std::find(_orderedCentroids.begin(), _orderedCentroids.end(), i));
    }

    // Compute angle of every neighbours
    neighbours = vector<vector<int> >(nbSuperpixels);
    neighboursAngle = vector<vector<float> >(nbSuperpixels);
    for (int i=0; i < nbSuperpixels; i++){
        Point centroid1(superpixels.getCentroids()[i].x, superpixels.getCentroids()[i].y);
        for (int j=0; j < nbSuperpixels; j++){
            if (_neighbours[i][j] && (i != j)){
                neighbours[i].push_back(j);
                Point centroid2(superpixels.getCentroids()[j].x, superpixels.getCentroids()[j].y);
                neighboursAngle[i].push_back(computeAngle(centroid1, centroid2));
            }
        }
    }

}

void SuperPatchMatcher::propagate(){
    int nbSuperpixels = superpixels1.getNbSuperpixels();
    int current;
    vector<bool> seen(nbSuperpixels, false);
    for (int i=0; i < nbSuperpixels; i++){
        current = orderedCentroids1[i];
        seen[current] = true;
        for (int j=0; j<neighbours1[i].size();j++) {
            int neighbour1 = neighbours1[i][j];
            float angle1 = neighboursAngle1[i][j];
            if (seen[neighbour1]) {
                float minAngleDiff = atan(1)*8;
                int idxMin = -1;
                for (int k=0; k<neighbours2[ANNs[neighbour1]].size(); k++){
                    
                }
            }
        }
    }
}

void SuperPatchMatcher::randomSearch(){
       

}