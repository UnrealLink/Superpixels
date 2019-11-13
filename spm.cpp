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