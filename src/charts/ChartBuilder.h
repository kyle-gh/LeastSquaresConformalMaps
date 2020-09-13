
#pragma once

#include <map>
#include <vector>
#include <queue>

#include "../util/MeshDef.h"

#include "../features/FeatureSet.h"

#include "Chart.h"
#include "FaceState.h"

using namespace Features;
using namespace Charts;

class ChartBuilder
{
private:
    Mesh* _mesh;
    
    const std::vector<FeatureSet>* _featureSets;
    
    std::map<Mesh::FaceHandle, FaceState> _faceStates;
    
    float _maxDistance;
    
    std::vector<std::vector<Mesh::FaceHandle>> _boundaries;
    
    std::map<Mesh::EdgeHandle, int> _chartBoundaries;
    
    std::vector<Chart> _charts;
    
public:
    ChartBuilder(Mesh* mesh, const std::vector<FeatureSet>* sets);
    
    const std::vector<Chart>& charts() const { return _charts;}

    void build();

    bool validate();
    
private:
    void findBoundaries();

    void buildCharts();

    void initializeBoundaries();
    bool expandBoundaries(int distance);
    
    void initializeCharts();
    
    bool removeChartBoundaryEdge(Mesh::EdgeHandle edge);
    bool checkChartBoundaryEdge(Mesh::EdgeHandle edge);
    
    void mergeCharts(Chart& chartTo, Chart& chartFrom);

    void splitCharts();
};
