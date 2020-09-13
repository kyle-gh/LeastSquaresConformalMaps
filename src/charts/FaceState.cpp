//
//  FaceState.cpp
//  LSCM
//
//  Created by Kyle on 5/6/18.
//  Copyright © 2018 Kyle. All rights reserved.
//

#include "FaceState.h"

namespace Charts
{
    FaceState::FaceState(Mesh::FaceHandle face, size_t numSets)
    : _face(face)
    , _setDistances(numSets, 0)
    , _distance(0.0f)
    , _touch(0)
    , _isBorder(false)
    , _chartIndex(SIZE_MAX)
    {
    }
    
    FaceState::FaceState()
    : FaceState(Mesh::FaceHandle(), 0)
    {}
    
    void FaceState::setSetDistance(int set, int distance)
    {
        _setDistances[set] = distance;
        
        for (size_t i = 0, size = _setDistances.size(); i < size; i++)
            if (_setDistances[i] != 0 && i != set)
                _isBorder = true;
    }
    
    bool FaceState::hasSetDistance(int set) const
    {
        return _setDistances[set] != 0;
    }
    
    void FaceState::calcDistance()
    {
        _distance = 0;
        
        for (int d : _setDistances)
        {
            _distance += (d * d);
        }
        
        _distance = sqrtf(_distance);
    }
    
    Mesh::FaceHandle FaceState::face() const
    {
        return _face;
    }
    
    int& FaceState::touch()
    {
        return _touch;
    }
    
    bool FaceState::isBorder() const
    {
        return _isBorder;
    }
    
    float FaceState::distance() const
    {
        return _distance;
    }
    
    bool FaceState::hasChart() const
    {
        return _chartIndex != SIZE_MAX;
    }
    
    size_t FaceState::chart() const
    {
        return _chartIndex;
    }
    
    void FaceState::setChart(size_t chart)
    {
        _chartIndex = chart;
    }
}
