//
//  Parameterizer.cpp
//  LSCM
//
//  Created by Kyle on 5/9/18.
//  Copyright © 2018 Kyle. All rights reserved.
//
//  Referenced:
//  Least Squares Conformal Maps for Automatic Texture PackingAtlas Generation - Levy et al
//  https://members.loria.fr/Bruno.Levy/papers/LSCM_SIGGRAPH_2002.pdf
//

#include "Parameterizer.h"

#include <iostream>

const float MIN_DEFAULT = FLT_MAX;
const float MAX_DEFAULT = -FLT_MAX;
const float THRESHOLD = 0.0000001f;

Parameterizer::Parameterizer(Mesh* mesh, const std::vector<Chart>* charts)
: _mesh(mesh)
, _charts(charts)
{
    _solver.setTolerance(THRESHOLD);
}

void Parameterizer::build()
{
    std::cout << "Parameterizing..." << std::endl;

    for (const auto& chart : *_charts)
    {
        build(chart);
    }
}

void Parameterizer::build(const Chart& chart)
{
    std::cout << "\tChart: " << chart.id() << std::endl;

    const auto numRows = chart.faces().size() * 2;

    std::cout << "\tSize: " << numRows << std::endl;

    setAnchors(chart);

    buildMaps(chart);

    _a.clear();
    _A.resize(numRows, _vmap.size() * 2);
    _A.setZero();

    _fa.clear();
    _fA.resize(numRows, _amap.size() * 2);
    _fA.setZero();

    _fx.resize(_fA.cols());
    _fx.setZero();

    _e.resize(_fA.rows());
    _e.setZero();

    setCoefficients(chart);

    _fA.setFromTriplets(_fa.begin(), _fa.end());
    _fA.makeCompressed();

    _A.setFromTriplets(_a.begin(), _a.end());
    _A.makeCompressed();

    _e = _fA * _fx;
    _e = _e * -1;

    _solver.setMaxIterations(_A.cols() * 5);
    _solver.compute(_A);

    _x = _solver.solve(_e);

    storeUVs(chart);
}

void Parameterizer::setAnchors(const Chart& chart)
{
    Mesh::Point a[2];
    
    findAxii(chart, a);

    auto minV = Mesh::VertexHandle();
    auto minU = MIN_DEFAULT;
    auto minUV = Mesh::Point();

    auto maxV = Mesh::VertexHandle();
    auto maxU = MAX_DEFAULT;
    auto maxUV = Mesh::Point();

    for (const auto& vertex : chart.vertices())
    {
        const auto& p = _mesh->point(vertex);
        
        const auto uv = Mesh::Point(p | a[0], p | a[1], 0);
        
        if (uv[0] < minU)
        {
            minV = vertex;
            minU = uv[0];
            minUV = uv;
        }
        
        if (uv[0] > maxU)
        {
            maxV = vertex;
            maxU = uv[0];
            maxUV = uv;
        }
    }

    _anchors = {{minUV, minV}, {maxUV, maxV}};
}

void Parameterizer::setCoefficients(const Chart& chart)
{
    Mesh::Point pv[3];
    VertexId vIds[3];

    for (const auto& face : chart.faces())
    {
        const auto realRow = _fmap[face];
        const auto imRow = realRow + 1;

        gatherAndProjectFace(face, pv, vIds);
        
        const auto pv01 = pv[1] - pv[0];
        const auto pv02 = pv[2] - pv[0];

        // Real and Imaginary
        setCoefficient(realRow, vIds[0], -pv01[0] + pv02[0], pv01[1] - pv02[1]);
        setCoefficient(imRow, vIds[0], -pv01[1] + pv02[1], -pv01[0] + pv02[0]);

        setCoefficient(realRow, vIds[1], -pv02[0], pv02[1]);
        setCoefficient(imRow, vIds[1], -pv02[1], -pv02[0]);

        setCoefficient(realRow, vIds[2], pv01[0], 0);
        setCoefficient(imRow, vIds[2], 0, pv01[0]);
    }

    for (const auto& anchor : _anchors)
    {
        const auto vId = id(anchor.h);

        _fx[vId.u] = anchor.uv[0];
        _fx[vId.v] = anchor.uv[1];
    }
}

void Parameterizer::setCoefficient(size_t row, const VertexId& vId, double u, double v)
{
    auto& a = isAnchor(vId.h) ? _fa : _a;

    a.emplace_back(row, vId.u, u);
    a.emplace_back(row, vId.v, v);
}

void Parameterizer::storeUVs(const Chart& chart)
{
    auto& texCoords = chart.texCoords();

    auto minUV = Mesh::TexCoord2D(MIN_DEFAULT, MIN_DEFAULT);
    auto maxUV = Mesh::TexCoord2D(MAX_DEFAULT, MAX_DEFAULT);
    
    for (const auto& vertex : chart.vertices())
    {
        auto vid = id(vertex);
        
        auto uv = Mesh::TexCoord2D();

        if (_amap.count(vertex) > 0)
        {
            auto anchor = getAnchor(vertex);
            uv = Mesh::TexCoord2D(anchor.uv[0], anchor.uv[1]);
        }
        else
        {
            uv = Mesh::TexCoord2D(_x(vid.u), _x(vid.v));
        }
        
        minUV.minimize(uv);
        maxUV.maximize(uv);
    }
    
    auto diff = maxUV - minUV;
    const auto length = diff.max();
    
    for (const auto& vertex : chart.vertices())
    {
        auto vid = id(vertex);

        auto uv = Mesh::TexCoord2D();

        if (_amap.count(vertex) > 0)
        {
            auto anchor = getAnchor(vertex);
            uv = Mesh::TexCoord2D(anchor.uv[0], anchor.uv[1]);
        }
        else
        {
            uv = Mesh::TexCoord2D(_x(vid.u), _x(vid.v));
        }
        
        const auto nuv = (uv - minUV) / length;

        _mesh->property(texCoords, vertex) = nuv;
        _mesh->set_texcoord2D(vertex, nuv);
    }
}

void Parameterizer::gatherAndProjectFace(const Mesh::FaceHandle& face, Mesh::Point* pv, Parameterizer::VertexId* vids)
{
    Mesh::Point v[3];
    
    auto i = 0;
    auto v_it = _mesh->fv_begin(face), v_end = _mesh->fv_end(face);
    for (; v_it != v_end; v_it++, i++)
    {
        v[i] = _mesh->point(*v_it);
        vids[i] = id(*v_it);
    }
    
    const auto v10 = v[1] - v[0];
    const auto v10Length = v10.length();
    const auto v20 = v[2] - v[0];
    
    const auto x = v10 / v10Length;
    const auto z = (x % v20).normalized();
    const auto y = z % x;
    
    pv[0] = Mesh::Point(0, 0, 0);
    pv[1] = Mesh::Point(v10Length, 0, 0);
    pv[2] = Mesh::Point(v20 | x, v20 | y, 0);
}

void Parameterizer::findAxii(const Chart& chart, Mesh::Point* a)
{
    auto min = Mesh::Point(MIN_DEFAULT, MIN_DEFAULT, MIN_DEFAULT);
    auto max = Mesh::Point(MAX_DEFAULT, MAX_DEFAULT, MAX_DEFAULT);
    
    for (const auto& vertex : chart.vertices())
    {
        const auto& p = _mesh->point(vertex);
        
        min.minimize(p);
        max.maximize(p);
    }
    
    auto diff = max - min;
    
    for (auto i = 0; i < 2; i++)
    {
        a[i] = Mesh::Point();
        
        auto v = diff.max();
        
        for (auto j = 0; j < 3; j++)
        {
            if (diff[j] == v)
            {
                a[i][j] = 1.0f;
                diff[j] = MAX_DEFAULT;
                break;
            }
        }
    }
}

void Parameterizer::buildMaps(const Chart& chart)
{
    _vmap.clear();
    _amap.clear();
    _fmap.clear();

    for (const auto& face : chart.faces())
    {
        if (_fmap.count(face) > 0)
        {
            continue;
        }

        const auto id = _fmap.size() * 2;
        _fmap[face] = id;
    }

    for (const auto& anchor : _anchors)
    {
        if (_amap.count(anchor.h) > 0)
        {
            continue;
        }

        const auto id = _amap.size() * 2;
        _amap[anchor.h] = id;
    }

    for (const auto& face : chart.faces())
    {
        auto fv_it = _mesh->fv_begin(face), v_end = _mesh->fv_end(face);
        for (; fv_it != v_end; fv_it++)
        {
            const auto& vertex = *fv_it;

            if (isAnchor(vertex))
            {
                continue;
            }

            if (_vmap.count(vertex) > 0)
            {
                continue;
            }

            const auto id = _vmap.size() * 2;
            _vmap[vertex] = id;
        }
    }
}

Parameterizer::VertexId Parameterizer::id(const Mesh::VertexHandle& vertex)
{
    VertexId id;
    id.u = isAnchor(vertex) ? _amap.at(vertex) : _vmap.at(vertex);
    id.v = id.u + 1;
    id.h = vertex;
    
    return id;
}

bool Parameterizer::isAnchor(const Mesh::VertexHandle& h)
{
    if (!_amap.empty())
    {
        return _amap.count(h) > 0;
    }

    return findAnchor(h) != _anchors.end();
}

const Parameterizer::Anchor& Parameterizer::getAnchor(const Mesh::VertexHandle& h)
{
    return *findAnchor(h);
}

Parameterizer::AnchorList::iterator Parameterizer::findAnchor(const Mesh::VertexHandle& h)
{
    return std::find_if(_anchors.begin(), _anchors.end(), [&h](const auto& anchor) { return anchor.h == h; });
}