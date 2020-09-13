
#pragma once

#include "../util/MeshDef.h"

namespace Features
{
    class FeatureMetric
    {
    public:
        virtual float eval(const Mesh* mesh, const Mesh::EdgeHandle& edge) { return 0.0f; }
        virtual bool eval(float value) { return false; }
        
        virtual float threshold() const { return 0.0f; }
        virtual void setThreshold(float t) {}
    };

    // Second Order Differences (SOD) - Angle between the normals
    class SODFeatureMetric : public FeatureMetric
    {
    private:
        float _threshold;
        
    public:
        SODFeatureMetric(float threshold);
        
        virtual float eval(const Mesh* mesh, const Mesh::EdgeHandle& edge);
        virtual bool eval(float value) { return value >= _threshold; }
        
        virtual float threshold() const { return _threshold; }
        virtual void setThreshold(float t) { _threshold = t; }
    };
}
