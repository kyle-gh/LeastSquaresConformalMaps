
#pragma once

#include "../util/MeshDef.h"

#include "FeatureMetric.h"

namespace Features
{
    class State
    {
    private:
        Mesh* _mesh;
        FeatureMetric& _metric;
        
        int _maxStringSize;
        int _minFeatureLength;
        
        FeatureValuePropHandle _featureValue;
        EdgeOwnerPropHandle _edgeOwner;
        
    public:
        State(Mesh* mesh, FeatureMetric& metric, int maxStringSize = 5, int minFeatureLength = 15);
        ~State();
        
        Mesh* mesh();
        
        FeatureMetric& metric();
        
        const FeatureValuePropHandle& featureValue() const;
        const EdgeOwnerPropHandle& edgeOwner() const;
        
        int maxStringLength() const;
        int minFeatureLength() const;
        
        Mesh::Scalar minFeatureValue() const;
        
        Mesh::Scalar edgeValue(const Mesh::HalfedgeHandle& halfedge) const;
        Mesh::Scalar edgeValue(const Mesh::EdgeHandle& edge) const;
        
        void setEdgeValue(const Mesh::EdgeHandle& edge, Mesh::Scalar value) const;
        
        bool hasOwner(const Mesh::HalfedgeHandle& halfedge) const;
        bool hasOwner(const Mesh::EdgeHandle& edge) const;
        
        void setOwner(const Mesh::HalfedgeHandle& halfedge, Mesh::Scalar owner);
        void setOwner(const Mesh::EdgeHandle& edge, Mesh::Scalar owner);
    };
}
