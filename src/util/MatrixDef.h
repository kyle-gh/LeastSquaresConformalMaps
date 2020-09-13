
#pragma once

#include <Eigen/IterativeLinearSolvers>
#include <Eigen/Dense>
#include <Eigen/Sparse>

typedef Eigen::Triplet<double> Triplet;
typedef std::vector<Triplet> TripletList;

typedef Eigen::SparseMatrix<double> SparseMatrix;

typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> MatrixX;
typedef Eigen::Matrix<double, Eigen::Dynamic, 1> MatrixXx1;
