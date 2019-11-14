#include "spm.h"
#include <stdlib.h>

SuperPatchMatcher::SuperPatchMatcher(Slic& _superpixels1, Slic& _superpixels2, int _degree) : 
    superpixels1(_superpixels1), superpixels2(_superpixels2) 
{
    degree = _degree;
    ANNs = vector<int>(superpixels1.getNbSuperpixels(), 0);
}

void SuperPatchMatcher::computeANNs(){
    initRandomANNs();


}

void SuperPatchMatcher::initRandomANNs(){
    int n = superpixels1.getNbSuperpixels();
    int m = superpixels2.getNbSuperpixels();
    if (m*degree < n) throw std::invalid_argument( "not enough superpixels in image 2 with current degree" );
    vector<int> count(m, 0); // Max assignment of a single superpixel as an ANN has to be less than degree
    srand (0); // initialize random seed
    for (int i=0; i<n; i++){
        ANNs[i] = rand() % m;
        while (count[ANNs[i]] >= degree){
           ANNs[i] = rand() % m; 
        }
        count[ANNs[i]]++;
    }
}

float computeAngle(Point p1, Point p2, int w, int h){
    return 0.;
}

void SuperPatchMatcher::computeOrderAndNeighbours(){
    int w = superpixels1.getImage().width();
    int h = superpixels1.getImage().height();
    int nbSuperpixels = superpixels1.getNbSuperpixels();
    vector<int> _orderedCentroids(nbSuperpixels, -1); // inverted vector of orders (label -> order)
    vector<vector<bool> > _neighbours(nbSuperpixels); // flag vector of neighbours
    for (int i=0; i < nbSuperpixels; i++){
        _neighbours[i] = vector<bool>(nbSuperpixels, false);
    }
    int k=0;
    for (int x=0; x < w; x++){
        for (int y=0; y < h; y++){
            // update order
            int label = superpixels1.getSuperpixels()(x, y);
            if (_orderedCentroids[label] == -1) {
                _orderedCentroids[label] = k;
                k++;
            }
            // Check neighbours
            if (x > 0 && !_neighbours[label][superpixels1.getSuperpixels()(x-1, y)]) {
                _neighbours[label][superpixels1.getSuperpixels()(x-1, y)] = true;
            }
            if (x < (w-1) && !_neighbours[label][superpixels1.getSuperpixels()(x+1, y)]) {
                _neighbours[label][superpixels1.getSuperpixels()(x+1, y)] = true;
            }
            if (y > 0 && !_neighbours[label][superpixels1.getSuperpixels()(x, y-1)]) {
                _neighbours[label][superpixels1.getSuperpixels()(x, y-1)] = true;
            }
            if (y < (h-1) && !_neighbours[label][superpixels1.getSuperpixels()(x, y+1)]) {
                _neighbours[label][superpixels1.getSuperpixels()(x, y+1)] = true;
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
        Point centroid1(superpixels1.getCentroids()[i].x, superpixels1.getCentroids()[i].y);
        for (int j=0; j < nbSuperpixels; j++){
            if (_neighbours[i][j] && (i != j)){
                neighbours[i].push_back(j);
                Point centroid2(superpixels1.getCentroids()[j].x, superpixels1.getCentroids()[j].y);
                neighboursAngle[i].push_back(computeAngle(centroid1, centroid2, w, h));
            }
        }
    }

}

void SuperPatchMatcher::propagate(){
    
    

}

void SuperPatchMatcher::randomSearch(){
       

}