
#pragma once

#include <vector>

#include "../util/MeshDef.h"
#include "../charts/Chart.h"

using namespace Charts;

class PackingChart
{
public:
    typedef std::pair<Mesh::TexCoord2D, Mesh::TexCoord2D> UVEdge;

    typedef std::vector<float> Horizon;

private:
    struct Data
    {
        Mesh::TexCoord2D min;
        Mesh::TexCoord2D max;
        std::vector<UVEdge> edges;

        Horizon top;
        Horizon bottom;
    };

private:
    const Chart *_chart;

    Mesh::TexCoord2D _position;

    Data _chartData;

    float _scale;
    Data _scaledChartData;

public:
    PackingChart(const Chart *chart);

    ~PackingChart() = default;

    const Chart *chart() const;

    void setPosition(const Mesh::TexCoord2D &p);

    Mesh::TexCoord2D position(bool scaled = true) const;

    float height(bool scaled = true) const;

    float width(bool scaled = true) const;

    size_t length(bool scaled = true) const;

    const Mesh::TexCoord2D &min(bool scaled = true) const;

    const Mesh::TexCoord2D &max(bool scaled = true) const;

    const Horizon &topHorizon(bool scaled = true) const;

    const Horizon &bottomHorizon(bool scaled = true) const;

    void build(const Mesh *mesh);

    void buildHorizons(float step, int padding);

private:
    const Data &chartData(bool scaled) const;

    void scale(float scale);

    void buildHorizons(float xStep, Data &chartData);
};
