#include "Geometry.h"

#include <cmath>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "Debug.h"
#include "Tracer/Ray.h"
#include "utils.h"

namespace RAYX {
Geometry::Geometry(GeometricalShape geometricShape, double width, double height,
                   const double azimuthalAngle, glm::dvec4 position,
                   glm::dmat4x4 orientation)
    : m_widthB(0), m_azimuthalAngle(azimuthalAngle) {
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
                   double widthB, double height, const double azimuthalAngle,
                   glm::dvec4 position, glm::dmat4x4 orientation)
    : m_widthB(widthB), m_azimuthalAngle(azimuthalAngle) {
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
    RAYX_LOG << "Calculated orientation";
    for (int i = 0; i < 4; i++) {
        std::stringstream s;
        s.precision(17);
        s << '\t';
        for (int j = 0; j < 4; j++) {
            s << orientation[i][j] << ", ";
        }
        RAYX_LOG << s.str();
    }
    std::stringstream s;
    s.precision(17);
    s << "Position: ";
    for (int i = 0; i < 4; i++) {
        s << position[i] << ", ";
    }
    RAYX_LOG << s.str();

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
    m_inMatrix = glmToArray16(glm::transpose(g2e));

    // inverse of m_inMatrix
    glm::dmat4x4 e2g = inv_rotation * inv_translation;
    m_outMatrix = glmToArray16(glm::transpose(e2g));

    /*RAYX_LOG << "from position and orientation";
    printDMatrix(m_inMatrix);
    printDMatrix(m_outMatrix);*/
}

void Geometry::getWidth(double& widthA, double& widthB) {
    widthA = m_widthA;
    widthB = m_widthB;
}

double Geometry::getHeight() { return m_height; }

double Geometry::getAzimuthalAngle() { return m_azimuthalAngle; }

std::array<double, 4 * 4> Geometry::getInMatrix() { return m_inMatrix; }

std::array<double, 4 * 4> Geometry::getOutMatrix() { return m_outMatrix; }

glm::dvec4 Geometry::getPosition() { return m_position; }

glm::dmat4x4 Geometry::getOrientation() { return m_orientation; }

void Geometry::setInMatrix(std::array<double, 4 * 4> inputMatrix) {
    m_inMatrix = inputMatrix;
}
void Geometry::setOutMatrix(std::array<double, 4 * 4> inputMatrix) {
    m_outMatrix = inputMatrix;
}

}  // namespace RAYX
