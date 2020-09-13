//
// Created by kyle on 8/26/20.
//

#include "PackingAtlas.h"

PackingAtlas::PackingAtlas(Mesh* mesh, float resolution, int padding)
: _mesh(mesh)
, _resolution(resolution)
, _dimension(0)
, _step(0)
, _padding(padding)
, _maxDimensions(0, 0)
{

}

float PackingAtlas::resolution() const
{
    return _resolution;
}

float PackingAtlas::dimension() const
{
    return _dimension;
}

const PackingChart::Horizon& PackingAtlas::horizon() const
{
    return _horizon;
}

bool PackingAtlas::pack(std::vector<PackingChart>& charts)
{
    for (auto &chart : charts)
    {
        _maxDimensions[0] += chart.width();
        _maxDimensions[1] += chart.height();
    }

    auto success = false;
    for (auto scale = 5.0f; scale > 1 && !success; scale -= 0.1f)
    {
        std::cout << "Scale: " << scale << std::endl;

        estimateDimension(scale);

        for (auto &chart : charts)
        {
            chart.buildHorizons(_step, _padding);

            success = false;

            auto minPosition = 0;
            auto minWasted = FLT_MAX;
            auto wastedSpace = 0.0f;

            if (chart.length() <= _horizon.size())
            {
                for (auto i = 0; i <= _horizon.size() - chart.length(); i++)
                {
                    if (estimateWastedSpace(i, chart, wastedSpace))
                    {
                        if (wastedSpace < minWasted)
                        {
                            minWasted = wastedSpace;
                            minPosition = i;
                            success = true;
                        }
                    }
                }
            }

            if (success)
            {
                chart.setPosition(mergeChart(minPosition, chart));
            }
            else
            {
                std::cout << "*Failed to pack Chart " << chart.chart()->id() << "..." << std::endl;
                break;
            }
        }
    }

    return success;
}

bool PackingAtlas::estimateWastedSpace(int x, const PackingChart& chart, float& wastedSpace)
{
    const auto& bottom = chart.bottomHorizon();

    auto maxY = FLT_MIN;

    for (auto i = 0, j = x; i < bottom.size(); i++, j++)
    {
        maxY = std::max(maxY, _horizon[j] + bottom[i]);
    }

    if (maxY >= _dimension)
    {
        wastedSpace = FLT_MAX;
        return false;
    }

    wastedSpace = 0;

    for (auto i = 0, j = x; i < bottom.size(); i++, j++)
    {
        const auto diff = maxY - bottom[i] - _horizon[j];

        wastedSpace += diff;
    }

    return true;
}

Mesh::TexCoord2D PackingAtlas::mergeChart(int x, const PackingChart& chart)
{
    auto maxY = FLT_MIN;

    const auto& top = chart.topHorizon();
    const auto& bottom = chart.bottomHorizon();

    for (auto i = 0, j = x; i < bottom.size(); i++, j++)
    {
        maxY = std::max(maxY, _horizon[j] + bottom[i]);
    }

    for (auto i = 0, j = x; i < bottom.size(); i++, j++)
    {
        _horizon[j] = maxY - top[i];
    }

    return {(float)x * _step, maxY};
}

void PackingAtlas::estimateDimension(float scale)
{
    _dimension = std::max(_maxDimensions[0], _maxDimensions[1]) / scale;

    _step = _dimension / _resolution;

    _horizon.resize(_dimension / _step);
    std::fill(_horizon.begin(), _horizon.end(), 0);
}