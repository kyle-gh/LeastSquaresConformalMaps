//
//  Chart.cpp
//  LSCM
//

#include "Chart.h"

#include "../util/MeshUtil.h"

namespace Charts
{
    size_t Chart::ChartsId = 1;

    Chart::Chart(Mesh* mesh)
    : _id(ChartsId++)
    , _maxDistance(0)
    , _isCleared(false)
    {
        if (mesh)
        {
            mesh->add_property(_texCoord);
        }

        _color = ColorCycler::Shared().next();
    }

    size_t Chart::id() const
    {
        return _id;
    }

    const Mesh::Color& Chart::color() const
    {
        return _color;
    }

    float Chart::maxDistance() const
    {
        return _maxDistance;
    }

    bool Chart::isCleared() const
    {
        return _isCleared;
    }

    const TexCoordPropHandle& Chart::texCoords() const
    {
        return _texCoord;
    }

    const FaceList& Chart::faces() const
    {
        return _faces;
    }

    const VertexList& Chart::vertices() const
    {
        return _vertices;
    }

    const VertexList& Chart::perimeter() const
    {
        return _perimeterVertices;
    }

    const EdgeList& Chart::perimeterEdges() const
    {
        return _perimeterEdges;
    }

    bool Chart::add(const Mesh::FaceHandle& fh, float distance)
    {
        if (contains(fh))
            return false;
        
        _faces.push_back(fh);
        
        _maxDistance = std::max(_maxDistance, distance);
        
        return true;
    }

    void Chart::merge(const Chart& other)
    {
        _faces.insert(_faces.end(), other._faces.begin(), other._faces.end());
        
        _maxDistance = std::max(_maxDistance, other._maxDistance);
    }

    void Chart::clear()
    {
        _faces.clear();
        _maxDistance = 0;
        _isCleared = true;
    }

    bool Chart::contains(const Mesh::FaceHandle& fh) const
    {
        return std::find(_faces.begin(), _faces.end(), fh) != _faces.end();
    }

    void Chart::setupReconstruction(Mesh* mesh, const FaceOwnerPropHandle& ownerProp, const VertexIdPropHandle& idProp, size_t& vertexId, VertexList& deleteVertices, FaceList& deleteFaces)
    {
        for (const auto& face : _faces)
        {
            mesh->property(ownerProp, face) = _id;
        }

        // Find perimeter faces and vertices
        for (const auto& face : _faces)
        {
            auto fe_it = mesh->cfe_begin(face), fe_end = mesh->cfe_end(face);
            for (; fe_it != fe_end; fe_it++)
            {
                const auto edge = *fe_it;

                auto isExternal = edge.is_boundary();
                for (auto i = 0; i < 2 && !isExternal; i++)
                {
                    isExternal = edge.halfedge(i).is_valid() && mesh->property(ownerProp, edge.halfedge(i).face()) != _id;
                }

                if (isExternal)
                {
                    _perimeterFaces.push_back(face);

                    for (auto i = 0; i < 2; i++)
                    {
                        _perimeterVertices.push_back(edge.v(i));
                    }
                }
            }
        }

        MeshUtil::Unique(_perimeterFaces);
        MeshUtil::Unique(_perimeterVertices);

        // Create chart-specific copies of perimeter vertices
        std::map<Mesh::VertexHandle, Mesh::VertexHandle> vertexMap;
        FaceList replaceFaces;

        for (const auto& vertex : _perimeterVertices)
        {
            auto copy = (Mesh::VertexHandle)mesh->add_vertex(Mesh::Point());
            mesh->copy_all_properties(vertex, copy, true);

            mesh->property(idProp, copy) = vertexId;
            vertexId++;

            mesh->status(copy).set_deleted(false);

            vertexMap[vertex] = copy;

            auto vf_iter = mesh->vf_begin(vertex), vf_end = mesh->vf_end(vertex);
            for (; vf_iter != vf_end; vf_iter++)
            {
                const auto& face = *vf_iter;

                if (contains(face))
                {
                    replaceFaces.push_back(face);
                }
            }

            deleteVertices.push_back(vertex);
        }

        MeshUtil::Unique(replaceFaces);

        // Create chart-specific copies of perimeter faces using the
        // newly created vertices.
        std::vector<Mesh::VertexHandle> faceVertices;
        faceVertices.reserve(3);

        for (const auto& face : replaceFaces)
        {
            faceVertices.clear();

            auto fv_iter = mesh->fv_ccwbegin(face), fv_end = mesh->fv_ccwend(face);
            for (; fv_iter != fv_end; fv_iter++)
            {
                const auto& vertex = *fv_iter;

                auto iter = vertexMap.find(vertex);
                if (iter == vertexMap.end())
                    faceVertices.push_back(vertex);
                else
                    faceVertices.push_back(iter->second);
            }

            IndexFace newFace = {
                mesh->property(idProp, faceVertices[0]),
                mesh->property(idProp, faceVertices[1]),
                mesh->property(idProp, faceVertices[2])
            };

            _reconstructFaces.push_back(newFace);

            deleteFaces.push_back(face);
        }
    }

    void Chart::reconstruct(Mesh* mesh, const FaceOwnerPropHandle& ownerProp, const std::map<size_t, Mesh::VertexHandle>& vertexMap)
    {
        // Rebuild faces list
        _faces.clear();

        for (const auto& face : mesh->faces())
        {
            if (mesh->property(ownerProp, face) == _id)
            {
                _faces.push_back(face);
            }
        }

        std::cout << "#Reconstructed: " << _reconstructFaces.size() << std::endl;

        // Recreated perimeter faces using the new chart-specific vertices
        _perimeterFaces.clear();

        for (const auto& face : _reconstructFaces)
        {
            Mesh::VertexHandle vertices[3];

            for (auto i = 0; i < 3; i++)
            {
                auto iter = vertexMap.find(face.ids[i]);
                if (iter == vertexMap.end())
                {
                    std::cout << "Failed to Find: " << face.ids[i] << std::endl;
                }
                else
                {
                    vertices[i] = iter->second;
                }
            }

            auto newFace = mesh->add_face(vertices, 3);

            _faces.push_back(newFace);

            auto isPerimeter = false;
            auto fe_it = mesh->cfe_begin(newFace), fe_end = mesh->cfe_end(newFace);
            for (; fe_it != fe_end && !isPerimeter; fe_it++)
            {
                isPerimeter = fe_it->is_boundary();
            }

            if (isPerimeter)
            {
                _perimeterFaces.push_back(newFace);
            }
        }

        // Rebuild vertex list
        _vertices.clear();

        for (const auto& face : _faces)
        {
            auto fv_iter = mesh->fv_begin(face), fv_end = mesh->fv_end(face);
            for (; fv_iter != fv_end; fv_iter++)
            {
                const auto vertex = *fv_iter;

                _vertices.push_back(vertex);
            }
        }

        MeshUtil::Unique(_vertices);

        // Rebuild perimeter edges and vertices
        _perimeterEdges.clear();
        _perimeterVertices.clear();

        for (const auto& face : _perimeterFaces)
        {
            auto fe_it = mesh->cfe_begin(face), fe_end = mesh->cfe_end(face);
            for (; fe_it != fe_end; fe_it++)
            {
                const auto& edge = *fe_it;

                if (edge.is_boundary())
                {
                    _perimeterEdges.push_back(edge);

                    _perimeterVertices.push_back(edge.v0());
                    _perimeterVertices.push_back(edge.v1());
                }
            }
        }

        MeshUtil::Unique(_perimeterEdges);
        MeshUtil::Unique(_perimeterVertices);
    }
}
