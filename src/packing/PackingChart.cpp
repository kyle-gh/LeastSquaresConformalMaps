//
// Created by kyle on 8/26/20.
//

#include "PackingChart.h"

PackingChart::PackingChart(const Chart* chart)
: _chart(chart)
, _scale(-1)
{
}

const Chart* PackingChart::chart() const
{
    return _chart;
}

Mesh::TexCoord2D PackingChart::position(bool scaled) const
{
    if (scaled)
        return _position;
    else
    {
        return _position + Mesh::TexCoord2D(
                (width(true) - width(false)) / 2,
                (height(true) - height(false)) / 2
        );
    }
}

void PackingChart::setPosition(const Mesh::TexCoord2D& p)
{
    _position = p;
}

float PackingChart::height(bool scaled) const
{
    return std::abs(chartData(scaled).max[1] - chartData(scaled).min[1]);
}

float PackingChart::width(bool scaled) const
{
    return std::abs(chartData(scaled).max[0] - chartData(scaled).min[0]);
}

size_t PackingChart::length(bool scaled) const
{
    return chartData(scaled).top.size();
}

const Mesh::TexCoord2D& PackingChart::min(bool scaled) const
{
    return chartData(scaled).min;
}

const Mesh::TexCoord2D& PackingChart::max(bool scaled) const
{
    return chartData(scaled).max;
}

const PackingChart::Horizon& PackingChart::topHorizon(bool scaled) const
{
    return chartData(scaled).top;
}

const PackingChart::Horizon& PackingChart::bottomHorizon(bool scaled) const
{
    return chartData(scaled).bottom;
}

void PackingChart::build(const Mesh* mesh)
{
    auto& texCoords = _chart->texCoords();

    _chartData.min = Mesh::TexCoord2D(FLT_MAX, FLT_MAX);
    _chartData.max = Mesh::TexCoord2D(FLT_MIN, FLT_MIN);

    for (const auto& edge : _chart->perimeterEdges())
    {
        auto to = mesh->property(texCoords, mesh->to_vertex_handle(mesh->halfedge_handle(edge, 0)));
        auto from = mesh->property(texCoords, mesh->from_vertex_handle(mesh->halfedge_handle(edge, 0)));

        if (to[0] > from[0])
        {
            std::swap(to, from);
        }

        _chartData.edges.emplace_back(to, from);

        _chartData.min.minimize(to);
        _chartData.max.maximize(to);

        _chartData.min.minimize(from);
        _chartData.max.maximize(from);
    }

    scale(0);
}

bool intersection(const Mesh::TexCoord2D& origin, const Mesh::TexCoord2D& ray, const Mesh::TexCoord2D& a, const Mesh::TexCoord2D& b, float& t)
{
    const auto v1 = origin - a;
    const auto v2 = b - a;
    const auto v3 = Mesh::TexCoord2D(-ray[1], ray[0]);

    const auto dot = v2 | v3;
    if (std::abs(dot) < FLT_EPSILON)
    {
        return false;
    }

    const auto t1 = ((v1[1] * v2[0] - (v1[0] * v2[1]))) / dot;
    const auto t2 = (v1 | v3) / dot;

    if (t1 >= 0 && (t2 >= 0 && t2 <= 1))
    {
        t = t1;
        return true;
    }

    return false;
}

void PackingChart::buildHorizons(float xStep, int padding)
{
    buildHorizons(xStep, _chartData);

    scale(xStep * (float)padding);
    buildHorizons(xStep, _scaledChartData);
}

void PackingChart::scale(float scale)
{
    if (_scale == scale)
    {
        return;
    }

    _scaledChartData.edges.clear();

    const auto center = (_chartData.max + _chartData.min) / 2.0f;

    scale = 1.0f + (scale / (center - _chartData.min).length());

    _scaledChartData.min = Mesh::TexCoord2D(FLT_MAX, FLT_MAX);
    _scaledChartData.max = Mesh::TexCoord2D(FLT_MIN, FLT_MIN);

    for (const auto& edge : _chartData.edges)
    {
        const auto from = (edge.first - center) * scale;
        const auto to = (edge.second - center) * scale;

        _scaledChartData.edges.emplace_back(from, to);

        _scaledChartData.min.minimize(to);
        _scaledChartData.max.maximize(to);

        _scaledChartData.min.minimize(from);
        _scaledChartData.max.maximize(from);
    }
}

void PackingChart::buildHorizons(float xStep, Data &chartData)
{
    chartData.top.clear();
    chartData.bottom.clear();

    const auto originStart = Mesh::TexCoord2D(chartData.min[0], chartData.min[1]);
    const auto originEnd = Mesh::TexCoord2D(chartData.max[0], chartData.min[1]);
    const auto ray = Mesh::TexCoord2D(0, 1);

    const auto step = Mesh::TexCoord2D(xStep, 0);
    float t;

    for (auto origin = originStart; origin[0] <= originEnd[0]; origin += step)
    {
        auto minT = FLT_MAX;
        auto maxT = FLT_MIN;

        for (const auto& edge : chartData.edges)
        {
            if (edge.first[0] <= origin[0] && origin[0] <= edge.second[0])
            {
                if (intersection(origin, ray, edge.first, edge.second, t))
                {
                    minT = std::min(minT, t);
                    maxT = std::max(maxT, t);
                }
            }
        }

        chartData.top.push_back(minT);
        chartData.bottom.push_back(maxT);
    }
}

const PackingChart::Data& PackingChart::chartData(bool scaled) const
{
    return scaled ? _scaledChartData : _chartData;
}