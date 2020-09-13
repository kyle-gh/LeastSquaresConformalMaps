//
//  Feature.cpp
//  LSCM
//
//  Created by Kyle on 4/11/18.
//  Copyright © 2018 Kyle. All rights reserved.
//

#include "Feature.h"

namespace Features
{
    Feature::Feature(const Mesh::EdgeHandle& edge, float value)
    : _edge(edge)
    , _value(value)
    {
        
    }
        
    float Feature::value() const
    {
        return _value;
    }
        
    const Mesh::EdgeHandle& Feature::edge() const
    {
        return _edge;
    }
    
    bool Feature::operator==(const Feature& other) const
    {
        return edge() == other.edge();
    }
    
    bool FeatureComparison_Value(const Feature& a, const Feature& b)
    {
        return a.value() > b.value();
    }
}
