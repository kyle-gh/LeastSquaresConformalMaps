//
//  main.cpp
//  LSCM
//
//  Created by Kyle
//  Copyright © 2020 Kyle. All rights reserved.
//

#include <iostream>

#include "features/FeatureBuilder.h"
#include "charts/ChartBuilder.h"
#include "parameterize/Parameterizer.h"
#include "packing/Packer.h"

#include "util/MeshUtil.h"
#include "util/VizUtil.h"
#include "util/Timing.h"

#include <cxxopts.hpp>

int main(int argc, char * argv[])
{
    cxxopts::Options options("lscm", "Generate texture atlas from a mesh");

    options.add_options()
            ("i,input", "Path to the input mesh", cxxopts::value<std::string>())
            ("o,output", "Path to save the output mesh", cxxopts::value<std::string>())
            ("v,viz", "Path to save visualizations", cxxopts::value<std::string>())
            ("r,resolution", "Resolution of packing texture", cxxopts::value<size_t>())
            ("p,padding", "Padding between charts", cxxopts::value<size_t>())
            ("val", "Validate output", cxxopts::value<bool>())
            ;

    std::string inputPath;
    std::string outputPath;
    std::string vizPath;
    size_t resolution = 2048;
    size_t padding = 4;
    std::stringstream path;
    bool validate = false;

    try
    {
        auto result = options.parse(argc, argv);

        if (!result.count("i") || !result.count("o"))
        {
            std::cout << options.help() << std::endl;
            exit(1);
        }

        inputPath = result["input"].as<std::string>();
        outputPath = result["output"].as<std::string>();

        if (result.count("v"))
        {
            vizPath = result["viz"].as<std::string>();
        }

        if (result.count("r"))
        {
            resolution = result["resolution"].as<size_t>();
        }

        if (result.count("p"))
        {
            padding = result["padding"].as<size_t>();
        }

        if (result.count("val"))
        {
            validate = result["val"].as<bool>();
        }
    }
    catch (const cxxopts::OptionException& e)
    {
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }


    auto mesh = MeshUtil::ReadMesh(inputPath, true);


    TIMER_START(Features);

    auto metric = SODFeatureMetric(15.0f);
    
    FeatureBuilder featureBuilder(
                  mesh.get(),
                  metric,
                  10,
                  20);
    
    featureBuilder.build();

    TIMER_END(Features);


    TIMER_START(Charts);

    ChartBuilder chartBuilder(mesh.get(), &featureBuilder.featureSets());

    chartBuilder.build();

    TIMER_END(Charts);

    if (validate)
    {
        if (!chartBuilder.validate())
        {
            std::cerr << "*Validation failed" << std::endl;
            return 1;
        }
    }

    if (!vizPath.empty())
    {
        VizUtil::PaintCharts(mesh, chartBuilder.charts());

        path.str("");
        path << vizPath << "/mesh-painted.ply";
        MeshUtil::Write(path.str(), mesh);
    }


    TIMER_START(Parameterization);

    Parameterizer param(mesh.get(), &chartBuilder.charts());

    param.build();

    TIMER_END(Parameterization);


    if (!vizPath.empty())
    {
        for (const auto &chart : chartBuilder.charts())
        {
            path.str("");
            path << vizPath << "/chart-" << chart.id() << ".bmp";
            VizUtil::DrawChart(path.str(), mesh, chart);
        }
    }


    TIMER_START(Packing);

    Packer packer(mesh.get(), &chartBuilder.charts(), resolution, padding);

    packer.pack();

    TIMER_END(Packing);


    if (!vizPath.empty())
    {
        for (const auto &chart : chartBuilder.charts())
        {
            path.str("");
            path << vizPath << "/chart-modified-" << chart.id() << ".bmp";
            VizUtil::DrawChart(path.str(), mesh, chart);

            path.str("");
            path << vizPath << "/chart-horizons-" << chart.id() << ".bmp";
            VizUtil::DrawChartHorizons(path.str(), packer.packingChart(chart.id()));
        }
    }


    TIMER_START(Apply);

    packer.apply();

    TIMER_END(Apply);


    if (!vizPath.empty())
    {
        path.str("");
        path << vizPath << "/atlas-horizons.bmp";
        VizUtil::DrawAtlasHorizons(path.str(), packer.atlas(), packer.packingCharts());

        path.str("");
        path << vizPath << "/atlas-horizon.bmp";
        VizUtil::DrawHorizon(path.str(), packer.atlas().horizon());

        path.str("");
        path << vizPath << "/atlas-uv.bmp";
        VizUtil::DrawAtlasUV(path.str(), mesh, chartBuilder.charts());
    }


    MeshUtil::Write(outputPath, mesh);

    return 0;
}

