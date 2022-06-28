#pragma once

#include <cmath>

#include <glm.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

// useful constants
const double inm2eV = 1.239852e3;
const double PI = 3.14159265358979323;

// useful functions
double RAYX_API hvlam(double x);
double RAYX_API degToRad(double degree);
double RAYX_API radToDeg(double rad);

glm::dmat4x4 getRotationMatrix(double dpsi, double dphi, double dchi);

void printDVec4(glm::dvec4 vec);
void RAYX_API printDMat4(glm::dmat4 matrix);
void printDMatrix(std::array<double, 4 * 4> matrix);
bool isIdentMatrix(glm::dmat4x4 matrix);

std::array<double, 4 * 4> glmToArray16(glm::dmat4x4 m);
glm::dmat4x4 RAYX_API arrayToGlm16(std::array<double, 4 * 4> m);

std::array<double, 4> glmToArray4(glm::dvec4 v);
glm::dvec4 RAYX_API arrayToGlm4(std::array<double, 4> v);

std::vector<double>::iterator movingAppend(std::vector<double>&& srcVector,
                                           std::vector<double>& destVector);
