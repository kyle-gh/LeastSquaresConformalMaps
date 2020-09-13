
#pragma once

#include <cstdio>

#include "MeshDef.h"

typedef std::unique_ptr<Mesh> MeshPtr;

class MeshUtil
{
public:
    static const float MAX_VALUE;
    static const float MIN_VALUE;

    static const Mesh::Point MIN_POINT;
    static const Mesh::Point MAX_POINT;

    static const Mesh::TexCoord2D MIN_TEXCOORD;
    static const Mesh::TexCoord2D MAX_TEXCOORD;

    static Mesh::Point AsPoint(const Mesh::TexCoord2D& t);

    template<typename T>
    static void Unique(std::vector<T>& list);

    static MeshPtr ReadMesh(const std::string& path, bool exitOnFail = false);
    static bool Write(const std::string& path, MeshPtr const& mesh);
    static bool Write(const std::string& path, const Mesh* mesh);
};

template<typename T>
void MeshUtil::Unique(std::vector<T>& list)
{
    std::sort(list.begin(), list.end());
    list.erase(
            std::unique(list.begin(), list.end()),
            list.end());
}
