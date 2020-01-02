#include "coloring.h"
#include <iostream>

Coloring::Coloring(Slic& _superpixels, vector<int> _filledClusters, vector<Vec3b> _colors) :
    superpixels(_superpixels), filledClusters(_filledClusters), colors(_colors)
{
    convertClustersToYUV();
    computeNeighbours(neighbours);
    computeAffinityMatrix(affinityMatrix);
    solveAffinityMatrix();
    convertClustersToLab();
}

Coloring::~Coloring(){}

void Coloring::convertClustersToYUV(){
    int nb = superpixels.getNbSuperpixels();
    clustersY = vector<int>(nb);
    Image<Vec3b> clusters(nb, 1);
    for (int i=0; i<nb; i++){
        clusters(i, 0) = Vec3b(superpixels.getCentroids()[i].L, superpixels.getCentroids()[i].a, superpixels.getCentroids()[i].b);
    }
    cvtColor(clusters, clusters, COLOR_Lab2BGR);
    cvtColor(clusters, clusters, COLOR_BGR2YUV);
    for (int i=0; i<nb; i++){
        clustersY[i] = clusters(i, 0)[0];
    }
    int nbFilled = colors.size();
    Image<Vec3b> clustersFilled(nbFilled, 1);
    for (int i=0; i<nbFilled; i++){
        clustersFilled(i, 0) = colors[i];
    }
    cvtColor(clustersFilled, clustersFilled, COLOR_BGR2YUV);
    for (int i=0; i<nbFilled; i++){
        colors[i] = clustersFilled(i, 0);
    }
}

void Coloring::computeNeighbours(vector<vector<int> >& neighbours){
    int w = superpixels.getImage().width();
    int h = superpixels.getImage().height();
    int nbSuperpixels = superpixels.getNbSuperpixels();
    vector<vector<bool> > _neighbours(nbSuperpixels); // flag vector of neighbours
    for (int i=0; i < nbSuperpixels; i++){
        _neighbours[i] = vector<bool>(nbSuperpixels, false);
    }

    for (int x=0; x < w; x++){
        for (int y=0; y < h; y++){
            int label = superpixels.getSuperpixels()(x, y);
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

    neighbours = vector<vector<int> >(nbSuperpixels);
    for (int i=0; i < nbSuperpixels; i++){
        for (int j=0; j < nbSuperpixels; j++){
            if (_neighbours[i][j] && (i != j)){
                neighbours[i].push_back(j);
            }
        }
    }
}

void Coloring::computeAffinityMatrix(SparseMatrix<float>& affinityMatrix){
    int nb = superpixels.getNbSuperpixels();
    affinityMatrix = SparseMatrix<float>(nb, nb);
    vector<Triplet<float> > coefficients;
    for (int r=0; r < nb; r++){
        if(std::find(filledClusters.begin(), filledClusters.end(), r) == filledClusters.end()) {
            vector<float> neighbourhood;
            neighbourhood.push_back((float)clustersY[r]);
            for (int neighbour : neighbours[r]){
                neighbourhood.push_back((float)clustersY[neighbour]);
            }
            float mu = computeMean(neighbourhood);
            float sigma = computeVar(neighbourhood, mu);
            float sum = 0;
            for (int s : neighbours[r]){
                sum += 1 + (clustersY[r]-mu)*(clustersY[s]-mu)/(sigma*sigma);
            }
            for (int s : neighbours[r]){
                coefficients.push_back(Triplet<float>(r, s, (- 1 - (clustersY[r]-mu)*(clustersY[s]-mu)/(sigma*sigma))/sum));
            }
        }
        coefficients.push_back(Triplet<float>(r, r, 1));
    }
    affinityMatrix.setFromTriplets(coefficients.begin(), coefficients.end());
}

void Coloring::solveAffinityMatrix(){
    int nb = superpixels.getNbSuperpixels();
    VectorXf bU = VectorXf::Zero(nb);
    VectorXf bV = VectorXf::Zero(nb);
    int j = 0;
    for (int i : filledClusters){
        bU[i] = colors[j][1];
        bV[i] = colors[j][2];
        j++;
    }

    BiCGSTAB<SparseMatrix<float> >  BCGST;
    BCGST.compute(affinityMatrix);
    VectorXf U = BCGST.solve(bU);
    VectorXf V = BCGST.solve(bV);

    newColors = vector<Vec3b>(nb);
    for (int i=0; i<nb; i++){
        // std::cout << i << " " << U[i] << std::endl;
        // std::cout << i << " " << V[i] << std::endl;
        newColors[i] = Vec3b(clustersY[i], U[i], V[i]);
    }
}

void Coloring::convertClustersToLab(){
    int nb = superpixels.getNbSuperpixels();
    Image<Vec3b> clusters(nb, 1);
    for (int i=0; i<nb; i++){
        clusters(i, 0) = newColors[i];
    }
    cvtColor(clusters, clusters, COLOR_YUV2BGR);
    cvtColor(clusters, clusters, COLOR_BGR2Lab);
    for (int i=0; i<nb; i++){
        newColors[i] = clusters(i, 0);
    }
}

float computeMean(const vector<float>& array){
    float mean = 0;
    if (array.size() == 0) {return mean;}
    for (float value : array){
        mean += value;
    }
    return mean/array.size();
}

float computeVar(const vector<float>& array, float mean){
    float var = 1;
    if (array.size() == 0) {return var;}
    for (float value : array){
        var += (value - mean)*(value - mean);
    }
    return var/array.size();
}