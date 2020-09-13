//
//  EdgeCriteria.cpp
//  LSCM
//
//  Created by Kyle on 4/11/18.
//  Copyright © 2018 Kyle. All rights reserved.
//

#include "FeatureMetric.h"

#include <cmath>
#include <iostream>

namespace Features
{
    SODFeatureMetric::SODFeatureMetric(Mesh::Scalar threshold)
    : _threshold(threshold)
    {
    }

    float SODFeatureMetric::eval(const Mesh* mesh, const Mesh::EdgeHandle& edge)
    {
        // Leverage OpenMesh's dihedral angle to estimate SOD
        return OpenMesh::rad_to_deg(std::fabs(mesh->calc_dihedral_angle_fast(edge)));
    }
}
