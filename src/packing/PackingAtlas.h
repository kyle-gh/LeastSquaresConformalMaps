
#pragma once

#include <map>

#include "PackingChart.h"

class PackingAtlas
{
private:
    Mesh* _mesh;

    float _resolution;
    float _dimension;
    float _step;

    int _padding;

    Mesh::TexCoord2D _maxDimensions;

    PackingChart::Horizon _horizon;

public:
    PackingAtlas(Mesh* mesh, float resolution = 2048.0f, int padding = 1);
    ~PackingAtlas() = default;

    float resolution() const;
    float dimension() const;

    const PackingChart::Horizon& horizon() const;

    bool pack(std::vector<PackingChart>& charts);

private:
    void estimateDimension(float scale);

    bool estimateWastedSpace(int x, const PackingChart& chart, float& wastedSpace);
    Mesh::TexCoord2D mergeChart(int x, const PackingChart& chart);
};
