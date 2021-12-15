#include "Geometry.h"

#include <math.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "Tracer/Ray.h"
#include "utils.h"

namespace RAYX {
Geometry::Geometry(GeometricalShape geometricShape, double width, double height,
                   glm::dvec4 position, glm::dmat4x4 orientation)
    : m_widthB(0) {
    m_geometricalShape = geometricShape;
    if (m_geometricalShape == GeometricalShape::ELLIPTICAL) {
        m_widthA = -width;
        m_height = -height;
    } else {
        m_widthA = width;
        m_height = height;
    }
    m_orientation = orientation;
    m_position = position;
    // calculate in and out matrices
    calcTransformationMatrices(position, orientation);
}

Geometry::Geometry(GeometricalShape geometricShape, double widthA,
                   double widthB, double height, glm::dvec4 position,
                   glm::dmat4x4 orientation)
    : m_widthB(widthB) {
    m_geometricalShape = geometricShape;
    if (m_geometricalShape == GeometricalShape::ELLIPTICAL) {
        m_widthA = -widthA;
        m_height = -height;
    } else {
        m_widthA = widthA;
        m_height = height;
    }
    // calculate in and out matrices
    calcTransformationMatrices(position, orientation);
}

Geometry::Geometry() {}
Geometry::~Geometry() {}

/**
 * calculates element to world coordinates transformation matrix and its inverse
 * @param   position     4 element vector which describes the position of the
 * element in world coordinates
 * @param   orientation  4x4 matrix that describes the orientation of the
 * surface with respect to the world coordinate system
 * @return void
 */
void Geometry::calcTransformationMatrices(glm::dvec4 position,
                                          glm::dmat4x4 orientation) {
    glm::dmat4x4 translation =
        glm::dmat4x4(1, 0, 0, -position[0], 0, 1, 0, -position[1], 0, 0, 1,
                     -position[2], 0, 0, 0, 1);  // o
    glm::dmat4x4 inv_translation =
        glm::dmat4x4(1, 0, 0, position[0], 0, 1, 0, position[1], 0, 0, 1,
                     position[2], 0, 0, 0, 1);  // o
    glm::dmat4x4 rotation = glm::dmat4x4(
        orientation[0][0], orientation[0][1], orientation[0][2], 0.0,
        orientation[1][0], orientation[1][1], orientation[1][2], 0.0,
        orientation[2][0], orientation[2][1], orientation[2][2], 0.0, 0.0, 0.0,
        0.0, 1.0);  // o
    glm::dmat4x4 inv_rotation = glm::transpose(rotation);

    // ray = tran * rot * ray
    glm::dmat4x4 g2e = translation * rotation;
    m_inMatrix = glmToVector16(glm::transpose(g2e));

    // inverse of m_inMatrix
    glm::dmat4x4 e2g = inv_rotation * inv_translation;
    m_outMatrix = glmToVector16(glm::transpose(e2g));

    std::cout << "from position and orientation" << std::endl;
    printMatrix(m_inMatrix);
    printMatrix(m_outMatrix);
}

void Geometry::getWidth(double& widthA, double& widthB) {
    widthA = m_widthA;
    widthB = m_widthB;
}

double Geometry::getHeight() { return m_height; }

std::vector<double> Geometry::getInMatrix() { return m_inMatrix; }

std::vector<double> Geometry::getOutMatrix() { return m_outMatrix; }

glm::dvec4 Geometry::getPosition() { return m_position; }

glm::dmat4x4 Geometry::getOrientation() { return m_orientation; }

void Geometry::setInMatrix(std::vector<double> inputMatrix) {
    assert(inputMatrix.size() == 16);
    m_inMatrix = inputMatrix;
}
void Geometry::setOutMatrix(std::vector<double> inputMatrix) {
    assert(inputMatrix.size() == 16);
    m_outMatrix = inputMatrix;
}

}  // namespace RAYX