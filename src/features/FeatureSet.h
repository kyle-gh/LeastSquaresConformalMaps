
#pragma once

#include "../util/MeshDef.h"

#include "Feature.h"

namespace Features
{
    class FeatureSet
    {
    private:
        static Mesh::Scalar _ids;
        
        static Mesh::Scalar nextId();
        
    private:
        Mesh::Scalar _id;
        
        std::vector<Mesh::HalfedgeHandle> _halfedges;
        
        Mesh::Scalar _totalValue;
        
    public:
        FeatureSet();
        
        Mesh::Scalar id() const;
        
        void clear();
        
        size_t size() const;
        
        bool isEmpty() const;
        
        std::vector<Mesh::HalfedgeHandle>& halfedges();
        const std::vector<Mesh::HalfedgeHandle>& halfedges() const;
        
        bool add(const Mesh::HalfedgeHandle& halfedge, Mesh::Scalar value);
        
        bool contains(const Mesh::HalfedgeHandle& halfedge) const;
    };
}
