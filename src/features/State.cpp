//
//  State.cpp
//  LSCM
//
//  Created by Kyle on 4/12/18.
//  Copyright © 2018 Kyle. All rights reserved.
//

#include "State.h"

namespace Features
{
    State::State(Mesh* mesh, FeatureMetric& metric, int maxStringSize, int minFeatureLength)
    : _mesh(mesh)
    , _metric(metric)
    , _maxStringSize(maxStringSize)
    , _minFeatureLength(minFeatureLength)
    {
        _mesh->request_face_normals();
        _mesh->update_face_normals();
        
        _mesh->add_property(_featureValue);
        _mesh->add_property(_edgeOwner);
    }
    
    State::~State()
    {
        _mesh->remove_property(_featureValue);
        _mesh->remove_property(_edgeOwner);
        
        _mesh->release_face_normals();
    }
    
    Mesh* State::mesh()
    {
        return _mesh;
    }
    
    FeatureMetric& State::metric()
    {
        return _metric;
    }
    
    const FeatureValuePropHandle& State::featureValue() const
    {
        return _featureValue;
    }
    
    const EdgeOwnerPropHandle& State::edgeOwner() const
    {
        return _edgeOwner;
    }
    
    int State::maxStringLength() const
    {
        return _maxStringSize;
    }
    
    int State::minFeatureLength() const
    {
        return _minFeatureLength;
    }
    
    Mesh::Scalar State::minFeatureValue() const
    {
        return 0.0f; //_metric.minTotalValue(_maxStringSize);
    }
    
    Mesh::Scalar State::edgeValue(const Mesh::HalfedgeHandle& halfedge) const
    {
        return edgeValue(_mesh->edge_handle(halfedge));
    }
    
    Mesh::Scalar State::edgeValue(const Mesh::EdgeHandle& edge) const
    {
        return _mesh->property(_featureValue, edge);
    }
    
    void State::setEdgeValue(const Mesh::EdgeHandle& edge, Mesh::Scalar value) const
    {
        _mesh->property(_featureValue, edge) = value;
    }
    
    bool State::hasOwner(const Mesh::HalfedgeHandle& halfedge) const
    {
        return hasOwner(_mesh->edge_handle(halfedge));
    }
    
    bool State::hasOwner(const Mesh::EdgeHandle& edge) const
    {
        return _mesh->property(_edgeOwner, edge) != 0;
    }
    
    void State::setOwner(const Mesh::HalfedgeHandle& halfedge, Mesh::Scalar owner)
    {
        setOwner(_mesh->edge_handle(halfedge), owner);
    }
    
    void State::setOwner(const Mesh::EdgeHandle& edge, Mesh::Scalar owner)
    {
        _mesh->property(_edgeOwner, edge) = owner;
    }
}
