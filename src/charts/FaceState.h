
#pragma once

#include "../util/MeshDef.h"

namespace Charts
{
    class FaceState
    {
    private:
        Mesh::FaceHandle _face;
        
        std::vector<int> _setDistances;
        float _distance;
        
        int _touch;
        
        bool _isBorder;
        
        size_t _chartIndex;
        
    public:
        FaceState(Mesh::FaceHandle face, size_t numSets);
        
        FaceState();
        
        void setSetDistance(int set, int distance);
        
        bool hasSetDistance(int set) const;
        
        void calcDistance();
        
        Mesh::FaceHandle face() const;
        
        int& touch();
        
        bool isBorder() const;
        
        float distance() const;
        
        bool hasChart() const;
        
        size_t chart() const;
        
        void setChart(size_t chart);
    };
}
