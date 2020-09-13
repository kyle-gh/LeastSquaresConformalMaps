
#pragma once

#include "../util/MatrixDef.h"

#include "../util/MeshDef.h"

#include "../charts/Chart.h"

using namespace Charts;

class Parameterizer
{
private:
    struct VertexId
    {
        size_t u;
        size_t v;
        Mesh::VertexHandle h;
    };

    struct Anchor
    {
        Mesh::Point uv;
        Mesh::VertexHandle h;
    };

    typedef std::map<Mesh::VertexHandle, size_t> VertexMap;
    typedef std::map<Mesh::FaceHandle, size_t> FaceMap;
    typedef std::vector<Anchor> AnchorList;
    
    Mesh* _mesh;
    const std::vector<Chart>* _charts;

    TripletList _a;
    SparseMatrix _A;
    MatrixXx1 _x;

    TripletList _fa;
    SparseMatrix _fA;
    MatrixXx1 _fx;

    MatrixXx1 _e;

    Eigen::LeastSquaresConjugateGradient<Eigen::SparseMatrix<double>> _solver;

    VertexMap _vmap;
    VertexMap _amap;
    FaceMap _fmap;

    AnchorList _anchors;
    
public:
    Parameterizer(Mesh* mesh, const std::vector<Chart>* charts);

    void build();

private:
    void build(const Chart& chart);

    void setAnchors(const Chart& chart);
    void setCoefficients(const Chart& chart);
    void setCoefficient(size_t row, const VertexId& vId, double u, double v);
    void storeUVs(const Chart& chart);
    
    void gatherAndProjectFace(const Mesh::FaceHandle& face, Mesh::Point* pv, VertexId* ids);
    
    void findAxii(const Chart& chart, Mesh::Point* a);
    
    void buildMaps(const Chart& chart);
    
    VertexId id(const Mesh::VertexHandle& vertex);

    bool isAnchor(const Mesh::VertexHandle& h);
    const Anchor& getAnchor(const Mesh::VertexHandle& h);
    AnchorList::iterator findAnchor(const Mesh::VertexHandle& h);
};
