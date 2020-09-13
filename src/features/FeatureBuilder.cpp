//
//  FeatureBuilder.cpp
//  LSCM
//
//  Created by Kyle on 4/11/18.
//  Copyright © 2018 Kyle. All rights reserved.
//
//  Referenced:
//  Least Squares Conformal Maps for Automatic Texture PackingAtlas Generation - Levy et al
//  https://members.loria.fr/Bruno.Levy/papers/LSCM_SIGGRAPH_2002.pdf

#include "FeatureBuilder.h"

#include <stack>
#include <vector>

using namespace Features;

FeatureBuilder::FeatureBuilder(Mesh* mesh, FeatureMetric& metric, int maxSetSize, int minFeatureLength)
: _state(mesh, metric, maxSetSize, minFeatureLength)
{
}

void FeatureBuilder::build()
{
    detect();
    
    expand();
}

void FeatureBuilder::detect(float ratio)
{
    auto mesh = _state.mesh();
    auto& metric = _state.metric();
    const auto numEdges = mesh->n_edges();
    
    _features.clear();
    _features.reserve(numEdges);
    
    _meshStatus.clear();
    
    std::cout << "Edges: " << numEdges << std::endl;
    
    for (auto e_it = mesh->edges_begin(), e_end = mesh->edges_end(); e_it != e_end; e_it++)
    {
        const auto& edge = *e_it;
        
        auto value = metric.eval(_state.mesh(), edge);
        
        _state.setEdgeValue(edge, value);
        
        _features.emplace_back(edge, value);
        
        for (auto i = 0; i < 2; i++)
            _meshStatus[mesh->halfedge_handle(*e_it, i)] = MS_None;
    }
    
    std::sort(_features.begin(), _features.end(), FeatureComparison_Value);
    
    auto numFeatures = 0;

    if (ratio > 0.0f)
        numFeatures = (int)(_features.size() * ratio);
    else
    {
        auto iter = std::find_if(
                     _features.begin(), _features.end(),
                     [&metric](const Feature& f) { return !metric.eval(f.value()); });
        
        if (iter != _features.end())
            numFeatures = (int)std::distance(_features.begin(), iter);
    }
    
    _features.resize(numFeatures);
    
    if (ratio > 0.0f)
        metric.setThreshold(_features.back().value());
    
    //std::reverse(_features.begin(), _features.end());
    
    std::cout << "features: " << numFeatures << std::endl;
    std::cout << "Threshold: " << metric.threshold() << std::endl;
}

void FeatureBuilder::expand()
{
    double totalFeatureSize = 0;
    
    auto* mesh = _state.mesh();
    
    FeatureSet featureSet;
    
    for (const auto& feature : _features)
    {
        size_t halfLengths[] = {0, 0};
        
        featureSet.clear();
        
        auto edge = feature.edge();
        
        for (auto i = 0; i < 2; i++)
        {
            expand(mesh->halfedge_handle(edge, i), featureSet);
            
            halfLengths[i] = featureSet.size();
        }
        
        std::sort(featureSet.halfedges().begin(), featureSet.halfedges().end());
        
        if (featureSet.size() > _state.minFeatureLength())
        {
            for (const auto& halfedge : featureSet.halfedges())
            {
                tagEdge(halfedge);
            }
            
            _featureSets.push_back(featureSet);
            
            totalFeatureSize += featureSet.size();
            
            std::cout << "Feature " << feature.edge() << " : " << featureSet.size() << " = " << halfLengths[0] << " + " << halfLengths[1] - halfLengths[0] << std::endl;
        }
    }
    
    std::cout << "Feature Sets: " << _featureSets.size() << std::endl;
    std::cout << "Avg. Size: " << (int)(totalFeatureSize / _featureSets.size()) << std::endl;
}

void FeatureBuilder::expand(const Mesh::HalfedgeHandle& halfedge, FeatureSet& featureSet)
{
    const auto maxStringThreshold = _state.maxStringLength() * _state.metric().threshold();
    
    auto mesh = _state.mesh();
    
    auto baseEdge = halfedge;
    
    std::vector<Mesh::HalfedgeHandle> path;
    
    std::stack<Mesh::HalfedgeHandle> dfs;
    
    //if (_meshStatus[halfedge] != MS_None)
    //    return;
    
    if (featureSet.isEmpty())
        featureSet.add(baseEdge, _state.edgeValue(baseEdge));
    
    do
    {
        path.clear();
        
        _searchStatus.clear();
        
        dfs.push(baseEdge);
        
        _searchStatus[baseEdge] = SearchStatus(
           mesh->calc_edge_vector(baseEdge).normalized(),
           _state.edgeValue(baseEdge));
        
        auto maxEdge = Mesh::HalfedgeHandle();
        auto maxValue = 0.0f;
        
        while (!dfs.empty())
        {
            auto parentEdge = dfs.top();
            dfs.pop();
            
            if (_meshStatus[parentEdge] != MS_None)
                continue;
            
            auto& parentStatus = _searchStatus[parentEdge];
            if (parentStatus.isTouched)
                continue;
            
            parentStatus.isTouched = true;
            
            if (parentStatus.depth >= _state.maxStringLength())
                continue;
            
            const auto& v = mesh->to_vertex_handle(parentEdge);
            
            for (auto e_it = mesh->voh_begin(v), e_end = mesh->voh_end(v); e_it != e_end; e_it++)
            {
                auto childEdge = *e_it;
                
                if (_meshStatus[childEdge] != MS_None)
                    continue;
                
                if (featureSet.contains(childEdge))
                    continue;
                
                auto hasStatus = _searchStatus.count(childEdge) > 0;
                auto childStatus = hasStatus ? _searchStatus[childEdge] : SearchStatus();
                
                if (_searchStatus[childEdge].isTouched)
                    continue;

                auto dir = mesh->calc_edge_vector(childEdge).normalized();
                
                if (isOpposite(dir, parentEdge))
                    continue;
                
                dfs.push(childEdge);

                childStatus.parent = parentEdge;
                childStatus.dir = dir;
                childStatus.valueTotal = parentStatus.valueTotal + _state.edgeValue(childEdge);
                childStatus.depth = parentStatus.depth + 1;
                
                _searchStatus[childEdge] = childStatus;
                
                if (childStatus.valueTotal > maxValue)
                {
                    maxEdge = childEdge;
                    maxValue = childStatus.valueTotal;
                }
            }
        }
        
        if (maxEdge == Mesh::HalfedgeHandle())
            break;
        
        traversePath(maxEdge, path);
        
        if (!path.empty())
        {
            baseEdge = path[1];
            
            featureSet.add(baseEdge, _state.edgeValue(baseEdge));
        }
    }
    while(_searchStatus[path.back()].valueTotal > maxStringThreshold);
}

bool FeatureBuilder::isOpposite(const Mesh::Normal& dir, const Mesh::HalfedgeHandle& parent)
{
    auto edge = parent;
    while(edge != Mesh::HalfedgeHandle())
    {
        const auto& status = _searchStatus[edge];
        
        auto cos_angle = OpenMesh::dot(dir, status.dir);
        auto angle = OpenMesh::rad_to_deg(std::acos(cos_angle));
        
        if (angle >= 90.0f)
            return true;
        
        edge = status.parent;
    }
    
    return false;
}

void FeatureBuilder::traversePath(const Mesh::HalfedgeHandle& edge, std::vector<Mesh::HalfedgeHandle>& path)
{
    auto e = edge;
    while (e != Mesh::HalfedgeHandle())
    {
        path.push_back(e);
        
        const auto& status = _searchStatus[e];
        
        e = status.parent;
    }
    
    std::reverse(path.begin(), path.end());
}

void FeatureBuilder::tagEdge(const Mesh::HalfedgeHandle& feature)
{
    const auto mesh = _state.mesh();
    
    _meshStatus[feature] = MS_Feature;
    
    const auto& edge = mesh->edge_handle(feature);
    
    for (auto i = 0; i < 2; i++)
    {
        const auto& halfedge = mesh->halfedge_handle(edge, i);
        
        if (_meshStatus[halfedge] == MS_None)
        {
            _meshStatus[halfedge] = MS_Neighbor;
        }
        
        tagNeighborhood(mesh->to_vertex_handle(halfedge));
        tagNeighborhood(mesh->from_vertex_handle(halfedge));
    }
}

void FeatureBuilder::tagNeighborhood(const Mesh::VertexHandle& vertex)
{
    const auto mesh = _state.mesh();
    
    for (auto v_it = mesh->cvv_begin(vertex), v_end = mesh->cvv_end(vertex); v_it != v_end; v_it++)
    {
        const auto& v = *v_it;
        
        for (auto e_it = mesh->cve_begin(v), e_end = mesh->cve_end(v); e_it != e_end; e_it++)
        {
            for (auto i = 0; i < 2; i++)
            {
                const auto& halfedge = mesh->halfedge_handle(*e_it, i);
                
                if (_meshStatus[halfedge] == MS_None)
                {
                    _meshStatus[halfedge] = MS_Neighbor;
                }
            }
        }
    }
}

void FeatureBuilder::paintMetric()
{
    auto mesh = _state.mesh();
    auto& metric = _state.metric();
    
    for(auto v_it = mesh->vertices_begin(), v_end = mesh->vertices_end(); v_it != v_end; v_it++)
    {
        Mesh::Scalar maxAngle = -1;
        for (auto e_it = mesh->ve_begin(*v_it), e_end = mesh->ve_end(*v_it); e_it != e_end; e_it++)
        {
            auto value = metric.eval(mesh, *e_it);
            
            maxAngle = std::max(maxAngle, value);
        }
        
        int c = (int)(255.0f * ((maxAngle / 90.0f)));
        mesh->set_color(*v_it, Mesh::Color(c, c, c));
    }
}

void FeatureBuilder::paintSets()
{
    ColorCycler colors;
    
    auto mesh = _state.mesh();
    
    for (const auto& featureSet : _featureSets)
    {
        auto color = colors.next();
        
        for (const auto& halfedge : featureSet.halfedges())
        {
            mesh->set_color(mesh->to_vertex_handle(halfedge), color);
            mesh->set_color(mesh->from_vertex_handle(halfedge), color);
        }
    }
}
