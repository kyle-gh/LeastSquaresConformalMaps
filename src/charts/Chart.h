
#pragma once

#include "../util/MeshDef.h"

namespace Charts
{
    class Chart
    {
    private:
        struct IndexFace
        {
            size_t ids[3];
        };

        static size_t ChartsId;

        size_t _id;
        Mesh::Color _color;

        FaceList _faces;

        VertexList  _vertices;

        VertexList  _perimeterVertices;

        EdgeList _perimeterEdges;

        FaceList _perimeterFaces;

        std::vector<IndexFace> _reconstructFaces;

        float _maxDistance;
        
        bool _isCleared;

        TexCoordPropHandle _texCoord;

    public:
        Chart(Mesh* mesh = nullptr);
        ~Chart() = default;

        size_t id() const;

        const Mesh::Color& color() const;

        float maxDistance() const;

        bool isCleared() const;

        const TexCoordPropHandle& texCoords() const;

        const FaceList& faces() const;

        const VertexList& vertices() const;

        const VertexList& perimeter() const;

        const EdgeList& perimeterEdges() const;

        bool add(const Mesh::FaceHandle& fh, float totalDistance);
        
        void merge(const Chart& other);

        void setupReconstruction(Mesh* mesh, const FaceOwnerPropHandle& ownerProp, const VertexIdPropHandle& idProp, size_t& vertexId, VertexList& deleteVertices, FaceList& deleteFaces);
        void reconstruct(Mesh* mesh, const FaceOwnerPropHandle& ownerProp, const std::map<size_t, Mesh::VertexHandle>& vertexMap);
        
        void clear();

    private:
        bool contains(const Mesh::FaceHandle& fh) const;
    };
}
