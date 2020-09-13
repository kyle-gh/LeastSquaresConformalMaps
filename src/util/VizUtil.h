
#pragma once

#include "MeshUtil.h"
#include "../charts/Chart.h"
#include "../packing/PackingChart.h"
#include "../packing/PackingAtlas.h"

class VizUtil
{
public:
    static void PaintCharts(MeshPtr const& mesh, const std::vector<Chart>& charts);

    static bool DrawChart(const std::string& path, MeshPtr const& mesh, const Charts::Chart& chart);

    static void DrawChartHorizons(const std::string& path, const PackingChart& chart);

    static void DrawHorizon(const std::string& path, const PackingChart::Horizon& horizon);

    static void DrawAtlasHorizons(const std::string& path, const PackingAtlas& atlas, const std::vector<PackingChart>& packingCharts);

    static void DrawAtlasUV(const std::string& path, MeshPtr const& mesh, const std::vector<Chart>& charts);
};
