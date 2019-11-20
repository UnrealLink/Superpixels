#include "spm.h"
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <opencv2/highgui/highgui.hpp>

SuperPatchMatcher::SuperPatchMatcher(Slic& _superpixels1, Slic& _superpixels2, int _degree) : 
    superpixels1(_superpixels1), superpixels2(_superpixels2) 
{
    degree = _degree;
    ANNs = vector<int>(superpixels1.getNbSuperpixels(), 0);
    reverseANNs = vector<list<int> >(superpixels2.getNbSuperpixels());
}

void SuperPatchMatcher::computeANNs(){
    initRandomANNs();
    computeOrderAndNeighbours(superpixels1, orderedCentroids1, neighbours1, neighboursAngle1);
    computeOrderAndNeighbours(superpixels2, orderedCentroids2, neighbours2, neighboursAngle2);
    for (int i=0; i < maxIter; i++){
        std::cout << "Etape " << i << std::endl;
        propagate();
        randomSearch();
    }
    showMatchs();
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
        reverseANNs[ANNs[i]].push_back(i);
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
    int offset = 0;
    for (int i=0; i < nbSuperpixels; i++){
        orderedCentroids[i] = std::distance(_orderedCentroids.begin(), std::find(_orderedCentroids.begin(), _orderedCentroids.end(), i));
        if (orderedCentroids[i] == orderedCentroids.size()){
            orderedCentroids[i] = std::distance(_orderedCentroids.begin(), std::find(_orderedCentroids.begin(), _orderedCentroids.end(), -1)) + offset;
            offset++;
        }
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

int sgn(float x){
    if (x < 0) return -1;
    return 1;
}

void SuperPatchMatcher::propagate(){
    int nbSuperpixels = superpixels1.getNbSuperpixels();
    int current;
    vector<bool> seen(nbSuperpixels, false);
    for (int i=0; i < nbSuperpixels; i++){
        current = orderedCentroids1[i]; // current index in unordered centroids list
        seen[current] = true;
        for (int j=0; j<neighbours1[current].size();j++) {
            int neighbour1 = neighbours1[current][j];
            if (seen[neighbour1]) {
                float angle1 = neighboursAngle1[current][j];
                float minAngleDiff = atan(1)*8;
                int idxMin = -1;
                // Finding neighbour of neighbour1 with the most similar orientation with current centroid
                for (int k=0; k<neighbours2[ANNs[neighbour1]].size(); k++){
                    float angleDiff = abs(neighboursAngle2[ANNs[neighbour1]][k] 
                                    - sgn(neighboursAngle2[ANNs[neighbour1]][k])*atan(1)*4 
                                    - angle1);
                    if (angleDiff > atan(1)*4) angleDiff = atan(1)*8 - angleDiff;
                    if (angleDiff < minAngleDiff) {
                        minAngleDiff = angleDiff;
                        idxMin = k;
                    }
                }
                if (idxMin == -1) continue;
                int newCandidate = neighbours2[ANNs[neighbour1]][idxMin];
                Centroid centroid1 = superpixels1.getCentroids()[current];
                Centroid centroidCurrentMatch = superpixels2.getCentroids()[ANNs[current]];
                Centroid centroidNewCandidate = superpixels2.getCentroids()[newCandidate];
                if (countMatchings[newCandidate] >= degree){
                    int swap;
                    float cost = cieLabDist(centroid1, centroidNewCandidate) - cieLabDist(centroid1, centroidCurrentMatch);
                    float minCost = cost + 1000000;
                    float possibleCost;
                    for (int possibleSwap : reverseANNs[newCandidate]){
                        Centroid centroidPossibleSwap = superpixels1.getCentroids()[possibleSwap];
                        possibleCost = cost + cieLabDist(centroidPossibleSwap, centroidCurrentMatch)
                                            - cieLabDist(centroidPossibleSwap, centroidNewCandidate);
                        if (possibleCost < minCost){
                            minCost = possibleCost;
                            swap = possibleSwap;
                        }
                    }
                    if (minCost < 0){
                        reverseANNs[ANNs[current]].remove(current);
                        reverseANNs[newCandidate].remove(swap);
                        ANNs[swap] = ANNs[current];
                        reverseANNs[ANNs[current]].push_back(swap);
                        ANNs[current] = newCandidate;
                        reverseANNs[newCandidate].push_back(current);
                    }
                }
                else {
                    if (cieLabDist(centroid1, centroidNewCandidate) < cieLabDist(centroid1, centroidCurrentMatch)){
                        reverseANNs[ANNs[current]].remove(current);
                        ANNs[current] = newCandidate;
                        reverseANNs[newCandidate].push_back(current);
                        countMatchings[newCandidate]++;
                    }
                }
            }
        }
    }
}

int SuperPatchMatcher::selectRandomSuperpixel(int d, int x, int y){
    int xMin = std::min(x-d, 0);
    int yMin = std::min(y-d, 0);
    int xMax = std::min(x+d, superpixels2.getImage().width());
    int yMax = std::min(y+d, superpixels2.getImage().height());
    int xRand = (rand() - xMin) % (xMax - xMin);
    int yRand = (rand() - yMin) % (yMax - yMin);
    return superpixels2.getSuperpixels()(xRand, yRand);
}

void SuperPatchMatcher::randomSearch(){
    for (int i=0; i<superpixels1.getNbSuperpixels(); i++){
        int current = orderedCentroids1[i];
        Centroid centroid1 = superpixels1.getCentroids()[current];
        int d = std::max(superpixels1.getImage().width(), superpixels1.getImage().height())/2;
        while (d > 2){
            Centroid centroidCurrentMatch = superpixels2.getCentroids()[ANNs[current]];
            int newCandidate = selectRandomSuperpixel(d, (int)centroidCurrentMatch.x, (int)centroidCurrentMatch.y);
            Centroid centroidNewCandidate = superpixels2.getCentroids()[newCandidate];
            if (countMatchings[newCandidate] >= degree){
                int swap;
                float cost = cieLabDist(centroid1, centroidNewCandidate) - cieLabDist(centroid1, centroidCurrentMatch);
                float minCost = cost + 1000000;
                float possibleCost;
                for (int possibleSwap : reverseANNs[newCandidate]){
                    Centroid centroidPossibleSwap = superpixels1.getCentroids()[possibleSwap];
                    possibleCost = cost + cieLabDist(centroidPossibleSwap, centroidCurrentMatch)
                                        - cieLabDist(centroidPossibleSwap, centroidNewCandidate);
                    if (possibleCost < minCost){
                        minCost = possibleCost;
                        swap = possibleSwap;
                    }
                }
                if (minCost < 0){
                    reverseANNs[ANNs[current]].remove(current);
                    reverseANNs[newCandidate].remove(swap);
                    ANNs[swap] = ANNs[current];
                    reverseANNs[ANNs[current]].push_back(swap);
                    ANNs[current] = newCandidate;
                    reverseANNs[newCandidate].push_back(current);
                }
            }
            else {
                if (cieLabDist(centroid1, centroidNewCandidate) < cieLabDist(centroid1, centroidCurrentMatch)){
                    reverseANNs[ANNs[current]].remove(current);
                    ANNs[current] = newCandidate;
                    reverseANNs[newCandidate].push_back(current);
                    countMatchings[newCandidate]++;
                }
            }
            d = d/2;
        }
    }
}

void SuperPatchMatcher::showMatchs() {
    int w = superpixels1.getImage().width();
    int h = superpixels1.getImage().height();
    Image<Vec3b> I(w, h);

    for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
            int i = superpixels1.getSuperpixels()(x, y);
            I(x, y)[0] = superpixels2.getCentroids()[ANNs[i]].L;
            I(x, y)[1] = superpixels2.getCentroids()[ANNs[i]].a;
            I(x, y)[2] = superpixels2.getCentroids()[ANNs[i]].b;
        }
    }

    cvtColor(I, I, COLOR_Lab2BGR);
    imshow("ANNs", I);
}
