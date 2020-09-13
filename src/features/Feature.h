
#pragma once

#include "../util/MeshDef.h"

namespace Features
{
    class Feature
    {
    private:
        Mesh::EdgeHandle _edge;
        float _value;
        
    public:
        Feature() = default;
        Feature(const Mesh::EdgeHandle& edge, float value);
        ~Feature() = default;
        
        float value() const;
        
        const Mesh::EdgeHandle& edge() const;
        
        bool operator==(const Feature& other) const;
    };
    
    bool FeatureComparison_Value(const Feature& a, const Feature& b);
}
