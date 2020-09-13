
#pragma once

#include "../util/MeshDef.h"
#include "../charts/Chart.h"
#include "PackingAtlas.h"

#include <cstdio>

class Packer
{
private:
    Mesh* _mesh;

    std::vector<PackingChart> _packingCharts;

    PackingAtlas _atlas;

public:
    Packer(Mesh* mesh, const std::vector<Charts::Chart>* charts, float resolution = 2048.0f, int padding = 4);

    const std::vector<PackingChart>& packingCharts() const;

    const PackingChart& packingChart(size_t id) const;

    const PackingAtlas& atlas() const;

    bool pack();

    void apply();

private:
    void scaling(const PackingChart& chart, float& scale);
    void rotation(const PackingChart& chart, float& theta, Mesh::TexCoord2D& center);

    void transformUV(const PackingChart& chart);
};
