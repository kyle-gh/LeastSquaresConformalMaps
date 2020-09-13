//
//  MeshUtil.cpp
//  LSCM
//
//  Created by Kyle on 5/6/18.
//  Copyright © 2018 Kyle. All rights reserved.
//

#include "MeshUtil.h"

const float MeshUtil::MAX_VALUE = FLT_MAX;
const float MeshUtil::MIN_VALUE = FLT_MIN;

const Mesh::Point MeshUtil::MIN_POINT = Mesh::Point(FLT_MIN, FLT_MIN, FLT_MIN);
const Mesh::Point MeshUtil::MAX_POINT = Mesh::Point(FLT_MAX, FLT_MAX, FLT_MAX);

const Mesh::TexCoord2D MeshUtil::MIN_TEXCOORD = Mesh::TexCoord2D(FLT_MIN, FLT_MIN);
const Mesh::TexCoord2D MeshUtil::MAX_TEXCOORD = Mesh::TexCoord2D(FLT_MAX, FLT_MAX);

Mesh::Point MeshUtil::AsPoint(const Mesh::TexCoord2D& t)
{
    return {t[0], t[1], 1};
}

MeshPtr MeshUtil::ReadMesh(const std::string& path, bool exitOnFail)
{
    auto mesh = std::make_unique<Mesh>();

    Mesh& meshRef = *mesh;

    meshRef.request_face_normals();
    meshRef.request_vertex_normals();
    meshRef.request_vertex_texcoords2D();

    OpenMesh::IO::Options opts;
    opts += OpenMesh::IO::Options::VertexNormal;
    opts += OpenMesh::IO::Options::FaceNormal;
    opts += OpenMesh::IO::Options::VertexTexCoord;

    if (!OpenMesh::IO::read_mesh(meshRef, path, opts))
    {
        std::cerr << "Failed to read mesh at [" << path << "]" << std::endl;

        if (exitOnFail)
            exit(1);

        return nullptr;
    }

    meshRef.update_face_normals();
    meshRef.update_vertex_normals();

    return mesh;
}

bool MeshUtil::Write(const std::string& path, MeshPtr const& mesh)
{
    return MeshUtil::Write(path, mesh.get());
}

bool MeshUtil::Write(const std::string& path, const Mesh* mesh)
{
    OpenMesh::IO::Options options = OpenMesh::IO::Options::VertexTexCoord;
    if (path.rfind(".ply") != std::string::npos)
    {
        options += OpenMesh::IO::Options::VertexColor;
    }

    if (!OpenMesh::IO::write_mesh(*mesh, path, options))
    {
        std::cerr << "Failed to write mesh to [" << path << "]" << std::endl;
        return false;
    }

    return true;
}

