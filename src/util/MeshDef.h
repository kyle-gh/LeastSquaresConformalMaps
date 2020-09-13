
#pragma once

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

typedef OpenMesh::TriMesh_ArrayKernelT<> Mesh;

typedef OpenMesh::EPropHandleT<Mesh::Scalar> FeatureValuePropHandle;
typedef OpenMesh::EPropHandleT<Mesh::Scalar> EdgeOwnerPropHandle;
typedef OpenMesh::FPropHandleT<size_t> FaceOwnerPropHandle;
typedef OpenMesh::VPropHandleT<size_t> VertexIdPropHandle;
typedef OpenMesh::VPropHandleT<Mesh::TexCoord2D> TexCoordPropHandle;

typedef std::vector<Mesh::FaceHandle> FaceList;
typedef std::vector<Mesh::VertexHandle> VertexList;
typedef std::vector<Mesh::EdgeHandle> EdgeList;

#include "ColorCycler.h"
