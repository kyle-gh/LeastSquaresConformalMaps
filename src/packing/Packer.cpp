//
//  Packer.cpp
//  LSCM
//

#include "Packer.h"

#include "../util/MeshUtil.h"

using namespace Charts;

Packer::Packer(Mesh* mesh, const std::vector<Chart>* charts, float resolution, int padding)
: _mesh(mesh)
, _atlas(mesh, resolution, padding)
{
    for (const auto& chart : *charts)
    {
        _packingCharts.emplace_back(&chart);
    }
}

const std::vector<PackingChart>& Packer::packingCharts() const
{
    return _packingCharts;
}

const PackingChart& Packer::packingChart(size_t id) const
{
    return *std::find_if(_packingCharts.begin(), _packingCharts.end(), [id](const PackingChart& c) { return c.chart()->id() == id; });
}

const PackingAtlas& Packer::atlas() const
{
    return _atlas;
}

bool Packer::pack()
{
    std::cout << "Packing Charts..." << std::endl;

    for (auto& chart : _packingCharts)
    {
        transformUV(chart);

        chart.build(_mesh);
    }

    std::sort(
        _packingCharts.begin(),
        _packingCharts.end(),
        [](const PackingChart& a, const PackingChart& b)
        {
            return b.height() < a.height();
            //return (b.height() * b.width()) < (a.height() * a.width());
        }
    );

    return _atlas.pack(_packingCharts);
}

void Packer::apply()
{
    std::cout << "Applying UVs..." << std::endl;

    auto maxUV = Mesh::TexCoord2D(FLT_MIN, FLT_MIN);

    auto start = _packingCharts.begin(), end = _packingCharts.end();

    for (const auto& chart : _packingCharts)
    {
        const auto position = chart.position(false);
        const auto min = chart.min(false);
        const auto max = chart.max(false);

        maxUV[0] = std::max(maxUV[0], position[0] + max[0]);
        maxUV[1] = std::max(maxUV[1], position[1] + min[1]);
    }

    for (const auto& chart : _packingCharts)
    {
        const auto position = chart.position(false);
        const auto min = chart.min(false);

        const auto& texCoords = chart.chart()->texCoords();

        auto minTex = Mesh::TexCoord2D(FLT_MAX, FLT_MAX);
        auto maxTex = Mesh::TexCoord2D(FLT_MIN, FLT_MIN);

        for (const auto& vertex : chart.chart()->vertices())
        {
            auto uv = _mesh->property(texCoords, vertex);
            uv[0] = (position[0] + uv[0]) / maxUV[0];
            uv[1] = (position[1] - uv[1]) / maxUV[1];

            _mesh->set_texcoord2D(vertex, uv);
        }
    }
}

void Packer::transformUV(const PackingChart& chart)
{
    auto& texCoords = chart.chart()->texCoords();

    auto scale = 1.0f;
    auto theta = 0.0f;
    auto center = Mesh::TexCoord2D();

    scaling(chart, scale);
    rotation(chart, theta, center);

    std::cout << "Chart: " << chart.chart()->id() << std::endl
        << "\tScale: " << scale << std::endl
        << "\tRotation: " << center << " :: " << theta << std::endl;

    const auto c = std::cos(theta);
    const auto s = std::sin(theta);

    auto minUV = Mesh::TexCoord2D(FLT_MAX, FLT_MAX);
    auto maxUV = Mesh::TexCoord2D(FLT_MIN, FLT_MIN);

    for (const auto vertex : chart.chart()->vertices())
    {
        auto uv = _mesh->property(texCoords, vertex);

        uv -= center;

        auto ruv = Mesh::TexCoord2D(
            (uv[0] * c) - (uv[1] * s),
            (uv[0] * s) + (uv[1] * c)
        );

        ruv *= scale;

        minUV.minimize(ruv);
        maxUV.maximize(ruv);

        _mesh->property(texCoords, vertex) = ruv;
    }

    for (const auto vertex : chart.chart()->vertices())
    {
        auto uv = _mesh->property(texCoords, vertex);

        uv -= minUV;

        _mesh->property(texCoords, vertex) = uv;
    }
}

void Packer::scaling(const PackingChart& chart, float& scale)
{
    auto& texCoords = chart.chart()->texCoords();

    auto uvArea = 0.0f;
    auto pointArea = 0.0f;

    for (const auto& face : chart.chart()->faces())
    {
        auto fv_it = _mesh->fv_begin(face);

        const auto& pP = _mesh->point(*fv_it);
        const auto& uvP = MeshUtil::AsPoint(_mesh->property(texCoords, *fv_it));
        ++fv_it;

        const auto& pQ = _mesh->point(*fv_it);
        const auto& uvQ = MeshUtil::AsPoint(_mesh->property(texCoords, *fv_it));
        ++fv_it;

        const auto& pR = _mesh->point(*fv_it);
        const auto& uvR = MeshUtil::AsPoint(_mesh->property(texCoords, *fv_it));

        pointArea += ((pQ - pP) % (pR - pP)).norm() * 0.5f;
        uvArea += ((uvQ - uvP) % (uvR - uvP)).norm() * 0.5f;
    }

    scale = std::max(1.0f, pointArea / uvArea);
}

void Packer::rotation(const PackingChart& chart, float& theta, Mesh::TexCoord2D& center)
{
    auto& texCoords = chart.chart()->texCoords();

    const auto& perimeter = chart.chart()->perimeter();

    auto diameter = std::array<int, 2>{0, 0};
    auto maxLengthSqr = 0.0;

    for (auto i = 0; i < perimeter.size(); i++)
    {
        const auto& pi = _mesh->property(texCoords, perimeter[i]);

        for (auto j = i + 1; j < perimeter.size(); j++)
        {
            const auto& pj = _mesh->property(texCoords, perimeter[j]);

            auto lengthSqr = (pi - pj).sqrnorm();
            if (lengthSqr > maxLengthSqr)
            {
                maxLengthSqr = lengthSqr;
                diameter[0] = i;
                diameter[1] = j;
            }
        }
    }

    const auto& p = _mesh->property(texCoords, perimeter[diameter[0]]);
    const auto& q = _mesh->property(texCoords, perimeter[diameter[1]]);
    const auto v = q - p;
    const auto d = v.normalized();

    theta = (d[0] < 0 ? -1.0f : 1.0f) * std::acos(d | Mesh::TexCoord2D(0, 1));
    center = p;
}

