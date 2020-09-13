//
//  FeatureString.cpp
//  LSCM
//
//  Created by Kyle on 4/11/18.
//  Copyright © 2018 Kyle. All rights reserved.
//

#include "FeatureSet.h"

namespace Features
{
    Mesh::Scalar FeatureSet::_ids = 0;
    
    Mesh::Scalar FeatureSet::nextId()
    {
        return _ids++;
    }
    
    FeatureSet::FeatureSet()
    : _id(nextId())
    , _totalValue(0)
    {
        
    }
    
    Mesh::Scalar FeatureSet::id() const
    {
        return _id;
    }

    void FeatureSet::clear()
    {
        _halfedges.clear();
        _totalValue = 0;
    }
    
    size_t FeatureSet::size() const
    {
        return _halfedges.size();
    }
    
    bool FeatureSet::isEmpty() const
    {
        return _halfedges.empty();
    }
    
    std::vector<Mesh::HalfedgeHandle>& FeatureSet::halfedges()
    {
        return _halfedges;
    }
    
    const std::vector<Mesh::HalfedgeHandle>& FeatureSet::halfedges() const
    {
        return _halfedges;
    }
    
    bool FeatureSet::add(const Mesh::HalfedgeHandle& halfedge, Mesh::Scalar value)
    {
        //if (contains(feature))
        //    return false;
        
        _halfedges.push_back(halfedge);
        
        _totalValue += value;
        
        return true;
    }

    bool FeatureSet::contains(const Mesh::HalfedgeHandle& halfedge) const
    {
        return std::find(_halfedges.begin(), _halfedges.end(), halfedge) != _halfedges.end();
    }
}
