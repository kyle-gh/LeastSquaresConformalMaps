//
// Created by kyle on 8/31/20.
//

#include "VizUtil.h"

#include <CImg.h>

using namespace cimg_library;

#include <fstream>

typedef CImg<unsigned char> Image;

const unsigned char ColorLine[3] = {0, 180, 180};
const unsigned char ColorPoint[3] = {255, 10, 10};
const unsigned char ColorFill = 50;

void DrawFaceUV(Image& img, MeshPtr const& mesh, const Mesh::FaceHandle& face, const unsigned char* color, bool invertY)
{
    const auto size = (float)img.width();

    Mesh::TexCoord2D uvs[3];
    auto index = 0;

    auto iter = mesh->cfv_begin(face), end = mesh->cfv_end(face);
    for (; iter != end; iter++)
    {
        auto uv = mesh->texcoord2D(*iter) * size;
        if (invertY)
        {
            uv[1] = size - uv[1];
        }

        uvs[index] = uv;
        index++;
    }

    img.draw_triangle(
            uvs[0][0], uvs[0][1],
            uvs[1][0], uvs[1][1],
            uvs[2][0], uvs[2][1],
            color, 1, ~0
    );
}

void DrawHorizonLine(Image& img, const PackingChart::Horizon& horizon, size_t step, const unsigned char* color)
{
    const auto minMax = std::minmax_element(horizon.begin(), horizon.end());

    const auto min = *minMax.first;
    const auto max = *minMax.second;

    auto prevX = 0;
    auto prevY = 0;

    for (auto i = 0, x = 0; i < horizon.size(); i += step, x++)
    {
        auto v = horizon[i];

        v = (v - min) / (max - min);
        v *= (float)img.width();

        auto y = std::round(v);

        if (i > 0)
        {
            img.draw_line(prevX, prevY, x, y, color, 1);

            //img.draw_point(x, y, ColorPoint, 1);
            //img.draw_point(x, y - 1, ColorPoint, 1);
        }

        prevX = x;
        prevY = y;
    }
}

void VizUtil::PaintCharts(MeshPtr const& mesh, const std::vector<Chart>& charts)
{
    mesh->request_vertex_colors();

    for (auto v_it = mesh->vertices_begin(), v_end = mesh->vertices_end(); v_it != v_end; v_it++)
    {
        mesh->set_color(*v_it, Mesh::Color(0, 0, 0));
    }

    for (auto& chart : charts)
    {
        if (chart.isCleared())
            continue;

        const auto color = chart.color();

        for (const auto &vertex : chart.vertices())
        {
            mesh->set_color(vertex, color);
        }
    }
}

bool VizUtil::DrawChart(const std::string& path, MeshPtr const& mesh, const Charts::Chart& chart)
{
    auto& texCoords = chart.texCoords();

    Image image(2048, 2048, 1, 3);
    image.fill(ColorFill);

    for (const auto& face : chart.faces())
    {
        DrawFaceUV(image, mesh, face, ColorLine, false);
    }

    for (const auto& vertex : chart.perimeter())
    {
        const auto uv = mesh->property(texCoords, vertex) * image.width();

        image.draw_circle(uv[0], uv[1], 2, ColorPoint, 1);
    }

    const auto& perimeter = chart.perimeter();

    auto diameter = std::array<int, 2>{0, 0};
    auto maxLengthSqr = 0.0;

    for (auto i = 0; i < perimeter.size(); i++)
    {
        const auto& pi = mesh->property(texCoords, perimeter[i]);

        for (auto j = i + 1; j < perimeter.size(); j++)
        {
            const auto& pj = mesh->property(texCoords, perimeter[j]);

            auto lengthSqr = (pi - pj).sqrnorm();
            if (lengthSqr > maxLengthSqr)
            {
                maxLengthSqr = lengthSqr;
                diameter[0] = i;
                diameter[1] = j;
            }
        }
    }

    const auto p = mesh->property(texCoords, perimeter[diameter[0]]) * image.width();
    const auto q = mesh->property(texCoords, perimeter[diameter[1]]) * image.width();

    image.draw_circle(p[0], p[1], 4, ColorPoint, 1);
    image.draw_circle(q[0], q[1], 4, ColorPoint, 1);

    image.save_bmp(path.c_str());

    return true;
}

void VizUtil::DrawChartHorizons(const std::string &path, const PackingChart &chart)
{
    Image image(512, 512, 1, 3);
    image.fill(ColorFill);

    const auto& top = chart.topHorizon();
    const auto& bottom = chart.bottomHorizon();

    auto topStep = std::max((size_t)1, top.size() / image.width());
    auto bottomStep = std::max((size_t)1, bottom.size() / image.width());

    const auto topMinMax = std::minmax_element(top.begin(), top.end());
    const auto bottomMinMax = std::minmax_element(bottom.begin(), bottom.end());

    auto min = std::min(*topMinMax.first, *bottomMinMax.first);
    auto max = std::max(*bottomMinMax.second, *topMinMax.second);

    auto prevX = 0;
    auto prevY = 0;

    for (auto i = 0, x = 0; i < top.size(); i += topStep, x++)
    {
        auto v = top[i];

        v = (v - min) / (max - min);
        v *= (float)image.width();
        auto y = std::round(v);

        if (i > 0)
        {
            image.draw_line(prevX, prevY, x, y, ColorLine, 1);
        }

        prevX = x;
        prevY = y;
    }

    for (auto i = 0, x = 0; i < bottom.size(); i += bottomStep, x++)
    {
        auto v = bottom[i];

        v = (v - min) / (max - min);
        v *= (float)image.width();
        auto y = std::round(v);

        if (i > 0)
        {
            image.draw_line(prevX, prevY, x, y, ColorPoint, 1);
        }

        prevX = x;
        prevY = y;
    }

    image.save_bmp(path.c_str());
}

void VizUtil::DrawHorizon(const std::string& path, const PackingChart::Horizon& horizon)
{
    Image image(2048, 2048, 1, 3);
    image.fill(ColorFill);

    const auto step = horizon.size() / (float)image.width();

    const auto max = *std::max_element(horizon.begin(), horizon.end());
    if (max > FLT_EPSILON)
    {
        const auto scale = (float)image.width() / max;

        auto prevX = -1;
        auto prevY = 0;

        for (auto i = 0, x = 0; i < horizon.size(); i += step, x++)
        {
            auto y = image.width() - horizon[i] * scale;

            image.draw_line(prevX, prevY, x, y, ColorPoint, 1);
            image.draw_line(prevX, prevY - 1, x, y - 1, ColorPoint, 1);

            prevX = x;
            prevY = y;
        }
    }

    image.save_bmp(path.c_str());
}

void VizUtil::DrawAtlasHorizons(const std::string& path, const PackingAtlas& atlas, const std::vector<PackingChart>& packingCharts)
{
    Image image(atlas.resolution(), atlas.resolution(), 1, 3);
    image.fill(ColorFill);

    const auto dimension = atlas.dimension();
    const auto step = atlas.dimension() / atlas.resolution();
    const auto scale = atlas.resolution() / atlas.dimension();

    for (const auto& chart : packingCharts)
    {
        const auto& position = chart.position(false);
        const auto& top = chart.topHorizon(false);
        const auto& bottom = chart.bottomHorizon(false);
        const auto length = chart.length(false);
        const auto color = chart.chart()->color().data();

        for (auto i = 0, x = (int)(position[0] / step); i < length; i++, x++)
        {
            const auto y1 = (dimension - (position[1] - top[i])) * scale;
            const auto y2 = (dimension - (position[1] - bottom[i])) * scale;

            image.draw_line(x, y1, x, y2, color, 1);
        }
    }

    image.save_bmp(path.c_str());
}

void VizUtil::DrawAtlasUV(const std::string& path, MeshPtr const& mesh, const std::vector<Chart>& charts)
{
    Image image(2048, 2048, 1, 3);
    image.fill(ColorFill);

    for (const auto& chart : charts)
    {
        const auto color = chart.color().data();

        for (const auto& face : chart.faces())
        {
            DrawFaceUV(image, mesh, face, color, true);
        }
    }

    image.save_bmp(path.c_str());
}