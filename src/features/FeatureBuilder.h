
#pragma once

#include <iostream>

#include <map>

#include "../util/MeshDef.h"

#include "State.h"

#include "Feature.h"
#include "FeatureSet.h"
#include "FeatureMetric.h"

using namespace Features;

class FeatureBuilder
{
private:
    struct SearchStatus
    {
        Mesh::HalfedgeHandle parent;
        Mesh::Normal dir;
        float valueTotal;
        int depth;
        bool isTouched;
        
        SearchStatus()
        : parent()
        , dir()
        , valueTotal(0)
        , depth(0)
        , isTouched(false)
        {}
        
        SearchStatus(const Mesh::Normal& d, float v)
        : SearchStatus()
        {
            dir = d;
            valueTotal = v;
        }
    };
    
    enum MeshStatus
    {
        MS_None,
        MS_Feature,
        MS_Neighbor,
    };
    
    State _state;
    
    std::map<Mesh::HalfedgeHandle, SearchStatus> _searchStatus;
    std::map<Mesh::HalfedgeHandle, MeshStatus> _meshStatus;
    
    std::vector<Feature> _features;
    std::vector<FeatureSet> _featureSets;
    
public:
    FeatureBuilder(Mesh* mesh, FeatureMetric& metric, int maxSetSize = 5, int minFeatureLength = 15);
    
    ~FeatureBuilder() = default;
    
    Mesh* mesh() { return _state.mesh(); }
    const std::vector<FeatureSet>& featureSets() const { return _featureSets; }

    void build();
    
    void detect(float ratio = -1.0f);
    
    void expand();
    
    void paintMetric();
    void paintSets();
    
private:
    void expand(const Mesh::HalfedgeHandle& halfedge, FeatureSet& featureSet);
    
    bool isOpposite(const Mesh::Normal& dir, const Mesh::HalfedgeHandle& parent);
    void traversePath(const Mesh::HalfedgeHandle& edge, std::vector<Mesh::HalfedgeHandle>& path);
    
    void tagEdge(const Mesh::HalfedgeHandle& edge);
    void tagNeighborhood(const Mesh::VertexHandle& vertex);
};
