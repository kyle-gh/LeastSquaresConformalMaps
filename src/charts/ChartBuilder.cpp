//
//  ChartBuilder.cpp
//  LSCM
//
//  Created by Kyle on 4/19/18.
//  Copyright © 2018 Kyle. All rights reserved.
//
//  Referenced:
//  Least Squares Conformal Maps for Automatic Texture PackingAtlas Generation - Levy et al
//  https://members.loria.fr/Bruno.Levy/papers/LSCM_SIGGRAPH_2002.pdf

#include "ChartBuilder.h"

#include <iostream>

#include "../util/MeshUtil.h"

ChartBuilder::ChartBuilder(Mesh* mesh, const std::vector<FeatureSet>* sets)
: _mesh(mesh)
, _featureSets(sets)
, _maxDistance(0.0f)
{
    const auto numSets = _featureSets->size();
    
    for (auto f_it = _mesh->faces_begin(), f_end = _mesh->faces_end(); f_it != f_end; f_it++)
    {
        _faceStates[*f_it] = FaceState(*f_it, numSets);
    }
}

void ChartBuilder::build()
{
    std::cout << "Building Charts..." << std::endl;

    findBoundaries();
    
    buildCharts();

    std::cout << "Charts: " << _charts.size() << std::endl;

    splitCharts();
}

void ChartBuilder::findBoundaries()
{
    initializeBoundaries();
    
    int depth = 2;
    while (expandBoundaries(depth))
        depth++;
    
    for (auto& iter : _faceStates)
    {
        iter.second.calcDistance();
        
        _maxDistance = std::max(_maxDistance, iter.second.distance());
    }
}

void ChartBuilder::initializeBoundaries()
{
    Mesh::VertexHandle vertices[2];
    
    const auto size = _featureSets->size();
    
    _boundaries.resize(size);
    
    for (auto i = 0; i < size; i++)
    {
        const auto& featureSet = _featureSets->at(i);
        auto& boundary = _boundaries[i];
        
        for (const auto& halfedge : featureSet.halfedges())
        {
            vertices[0] = _mesh->to_vertex_handle(halfedge);
            vertices[1] = _mesh->from_vertex_handle(halfedge);
            
            for (const auto& vertex : vertices)
            {
                for (auto f_it = _mesh->vf_begin(vertex), f_end = _mesh->vf_end(vertex); f_it != f_end; f_it++)
                {
                    const auto& face = *f_it;
                    
                    if (std::find(boundary.begin(), boundary.end(), face) == boundary.end())
                    {
                        boundary.push_back(face);
                        
                        _faceStates[face].setSetDistance(i, 1);
                    }
                }
            }
        }
    }
}

bool ChartBuilder::expandBoundaries(int distance)
{
    bool wasModified = false;
    
    std::vector<Mesh::FaceHandle> newBoundary;
    
    for (auto i = 0; i < _boundaries.size(); i++)
    {
        auto& boundary = _boundaries[i];
        auto b_begin = boundary.begin();
        auto b_end = boundary.end();
        newBoundary.clear();
        
        for (const auto& face : boundary)
        {
            if (_faceStates[face].isBorder())
                continue;
            
            for (auto f_it = _mesh->ff_begin(face), f_end = _mesh->ff_end(face); f_it != f_end; f_it++)
            {
                auto& state = _faceStates[*f_it];
                
                if (state.hasSetDistance(i))
                    continue;
                
                if (std::find(b_begin, b_end, *f_it) != b_end)
                    continue;
                
                state.setSetDistance(i, distance);
                
                newBoundary.push_back(*f_it);
            }
        }
        
        boundary.resize(newBoundary.size());
        
        if (!newBoundary.empty())
        {
            std::copy(newBoundary.begin(), newBoundary.end(), boundary.begin());
            wasModified = true;
        }
    }
    
    return wasModified;
}

void ChartBuilder::buildCharts()
{
    auto he_compare = [this](const Mesh::HalfedgeHandle& a, const Mesh::HalfedgeHandle& b) -> bool
    {
        const auto& stateA = _faceStates[_mesh->face_handle(a)];
        const auto& stateB = _faceStates[_mesh->face_handle(b)];
        
        return stateA.distance() < stateB.distance();
    };
    
    std::priority_queue<Mesh::HalfedgeHandle, std::vector<Mesh::HalfedgeHandle>, decltype(he_compare)> heap(he_compare);
    
    const auto epsDistance = _maxDistance / 4;
    
    initializeCharts();
    
    for (const auto& chart : _charts)
    {
        const auto face = chart.faces()[0];
        
        for (auto he_it = _mesh->fh_begin(face), he_end = _mesh->fh_end(face); he_it != he_end; he_it++)
        {
            heap.push(*he_it);
        }
    }

    for (auto e_it = _mesh->edges_begin(), e_end = _mesh->edges_end(); e_it != e_end; e_it++)
    {
        _chartBoundaries[*e_it] = 1;
    }
    
    while (!heap.empty())
    {
        auto halfedge = heap.top();
        heap.pop();
        
        auto edge = _mesh->edge_handle(halfedge);
        
        auto face = _mesh->face_handle(halfedge);
        auto faceOpp = (Mesh::FaceHandle)_mesh->opposite_face_handle(halfedge);

        if (!face.is_valid() || !faceOpp.is_valid())
        {
            continue;
        }

        auto* state = &_faceStates[face];
        auto* stateOpp = &_faceStates[faceOpp];

        if (!state->hasChart())
        {
            std::swap(face, faceOpp);
            std::swap(state, stateOpp);
        }
        
        if (!stateOpp->hasChart())
        {
            auto& chart = _charts[state->chart()];
            
            chart.add(faceOpp, stateOpp->distance());
            
            stateOpp->setChart(state->chart());
            
            removeChartBoundaryEdge(edge);
            
            for (auto e_it = _mesh->fe_begin(faceOpp), e_end = _mesh->fe_end(faceOpp); e_it != e_end; e_it++)
            {
                auto e = *e_it;
                
                if (_chartBoundaries.count(e) > 0)
                {
                    heap.push(_mesh->halfedge_handle(e, 0));
                    heap.push(_mesh->halfedge_handle(e, 1));
                }
            }
        }
        else if (state->chart() != stateOpp->chart())
        {
            auto& chart = _charts[state->chart()];
            auto& chartOpp = _charts[stateOpp->chart()];
            
            if ((chart.maxDistance() - state->distance()) < epsDistance && (chartOpp.maxDistance() - state->distance()) < epsDistance)
            {
                mergeCharts(chart, chartOpp);
            }
        }
    }

    auto last = std::remove_if(_charts.begin(), _charts.end(), [](const Chart& chart) {
        return chart.isCleared() || chart.faces().empty();
    });

    _charts.erase(last, _charts.end());
}

void ChartBuilder::splitCharts()
{
    // Request 'status' attributes so geometry can be deleted
    _mesh->request_face_status();
    _mesh->request_edge_status();
    _mesh->request_vertex_status();
    _mesh->request_halfedge_status();

    // Create vertex-id and face-owner properties to
    // track vertices and faces after deletion and garbage collection.
    // This process invalidates all handles.
    VertexIdPropHandle vertexIdProp;
    _mesh->add_property(vertexIdProp);

    size_t vertexId = 0;

    for (const auto& vertex : _mesh->vertices())
    {
        _mesh->property(vertexIdProp, vertex) = vertexId;
        vertexId++;
    }

    FaceOwnerPropHandle faceOwnerProp;
    _mesh->add_property(faceOwnerProp);

    VertexList vertexDelete;
    FaceList faceDelete;

    for (auto& chart : _charts)
    {
        chart.setupReconstruction(_mesh, faceOwnerProp, vertexIdProp, vertexId, vertexDelete, faceDelete);
    }

    MeshUtil::Unique(vertexDelete);
    MeshUtil::Unique(faceDelete);

    std::cout << "Reconstructing: " << vertexDelete.size() << " vertices, " << faceDelete.size() << " faces" << std::endl;

    for (const auto& vertex : vertexDelete)
    {
        _mesh->status(vertex).set_deleted(true);
    }

    for (const auto& face : faceDelete)
    {
        _mesh->delete_face(face, false);
    }

    _mesh->garbage_collection();

    std::map<size_t, Mesh::VertexHandle> vertexMap;
    for (const auto& vertex : _mesh->vertices())
    {
        vertexMap[_mesh->property(vertexIdProp, vertex)] = (Mesh::VertexHandle)vertex;
    }

    for (auto& chart : _charts)
    {
        chart.reconstruct(_mesh, faceOwnerProp, vertexMap);
    }

    _mesh->remove_property(vertexIdProp);
    _mesh->remove_property(faceOwnerProp);
}

bool ChartBuilder::validate()
{
    std::map<Mesh::VertexHandle, int> touched;

    // Check if all vertices are present in charts
    for (const auto& vertex : _mesh->all_vertices())
    {
        touched[vertex] = 0;
    }

    for (const auto& chart : _charts)
    {
        for (const auto& vertex : chart.vertices())
        {
            touched[vertex]++;
        }
    }

    auto iter = touched.begin(), end = touched.end();
    for (; iter != end; iter++)
    {
        if (iter->second == 0)
        {
            std::cout << "Vertex " << iter->first << " not in a chart" << std::endl;
            return false;
        }
    }

    // Check if any of the charts are duplicates or overlap significantly.
    for (auto i = 0; i < _charts.size(); i++)
    {
        const auto& chartA = _charts[i];

        touched.clear();

        for (const auto& vertex : chartA.vertices())
        {
            touched[vertex] = 1;
        }

        for (auto j = i + 1; j < _charts.size(); j++)
        {
            const auto& chartB = _charts[j];

            auto count = 0;

            for (const auto& vertex : chartB.vertices())
            {
                if (touched.find(vertex) != touched.end())
                {
                    count++;
                }
            }

            if (count > chartB.perimeter().size())
            {
                std::cout << "Charts " << chartA.id() << " and " << chartB.id() << " overlap at " << count << " vertices > " << chartB.perimeter().size() << std::endl;

                return false;
            }
        }
    }

    return true;
}

void ChartBuilder::initializeCharts()
{
    auto f_compare = [this](const Mesh::FaceHandle& a, const Mesh::FaceHandle& b) -> bool
    {
        const auto& stateA = _faceStates[a];
        const auto& stateB = _faceStates[b];
        
        return stateA.distance() < stateB.distance();
    };
    
    std::priority_queue<Mesh::FaceHandle, std::vector<Mesh::FaceHandle>, decltype(f_compare)> heap(f_compare);
    
    for (const auto& iter : _faceStates)
    {
        if (iter.second.isBorder())
            heap.push(iter.second.face());
    }
    
    const auto size = _featureSets->size();
    
    std::vector<int> sets;
    sets.resize(size, 0);
    
    while(!heap.empty())
    {
        auto face = heap.top();
        heap.pop();
        
        auto& state = _faceStates[face];
        
        bool hasUnused = false;
        for (auto i = 0; i < size; i++)
            if (sets[i] == 0 && state.hasSetDistance(i))
                hasUnused = true;
        
        if (hasUnused)
        {
            for (auto i = 0; i < size; i++)
                if (state.hasSetDistance(i))
                    sets[i] = 1;

            state.setChart(_charts.size());

            _charts.emplace_back(_mesh);
            _charts.back().add(face, state.distance());
        }
    }
}

bool ChartBuilder::removeChartBoundaryEdge(Mesh::EdgeHandle edge)
{
    if (_chartBoundaries.count(edge) == 0)
        return false;
    
    _chartBoundaries.erase(edge);
    
    auto halfedge = _mesh->halfedge_handle(edge, 0);
    
    Mesh::VertexHandle vertices[2];
    vertices[0] = _mesh->to_vertex_handle(halfedge);
    vertices[1] = _mesh->from_vertex_handle(halfedge);
    
    for (const auto& vertex : vertices)
    {
        for (auto e_it = _mesh->cve_begin(vertex), e_end = _mesh->cve_end(vertex); e_it != e_end; )
        {
            auto e = *e_it;
            
            if (!checkChartBoundaryEdge(e) && removeChartBoundaryEdge(e))
            {
                e_it = _mesh->cve_begin(vertex);
            }
            else
            {
                e_it++;
            }
        }
    }
    
    return true;
}

bool ChartBuilder::checkChartBoundaryEdge(Mesh::EdgeHandle edge)
{
    if (_chartBoundaries.count(edge) == 0)
        return false;
    
    auto halfedge = _mesh->halfedge_handle(edge, 0);
    
    Mesh::VertexHandle vertices[2];
    vertices[0] = _mesh->to_vertex_handle(halfedge);
    vertices[1] = _mesh->from_vertex_handle(halfedge);
    
    int connections = 0;
    
    for (const auto& vertex : vertices)
    {
        for (auto e_it = _mesh->cve_begin(vertex), e_end = _mesh->cve_end(vertex); e_it != e_end; e_it++)
        {
            if (_chartBoundaries.count(*e_it) > 0)
            {
                connections++;
                break;
            }
        }
    }
    
    return connections == 2;
}

void ChartBuilder::mergeCharts(Chart& chartTo, Chart& chartFrom)
{
    assert(!chartTo.isCleared());
    assert(!chartFrom.isCleared());
    
    const auto& faceRef = chartTo.faces()[0];
    auto chartIndex = _faceStates[faceRef].chart();
    
    for (const auto& f : chartFrom.faces())
    {
        _faceStates[f].setChart(chartIndex);
    }
    
    chartTo.merge(chartFrom);
    
    chartFrom.clear();
}


