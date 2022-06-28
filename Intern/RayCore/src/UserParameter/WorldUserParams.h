#pragma once

#include <cmath>

#include <array>
#include <glm.hpp>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "Core.h"
#include "Model/Beamline/OpticalElement.h"
#include "Tracer/Ray.h"
#include "utils.h"

namespace RAYX {
class RAYX_API WorldUserParams {
  public:
    WorldUserParams(double alpha, double beta, double chi, double dist,
                    std::array<double, 6> misalignment,
                    double tangentAngle = 0);
    WorldUserParams();
    ~WorldUserParams();

    /**
     * this is the element to beam coordinate matrix that is needed to derive
     * the world coordinates of an element based on those of the previous
     * element
     */
    glm::dmat4x4 calcE2B();
    glm::dmat4x4 getMisalignmentOrientation();
    /** this is used only for the ellipsoid where two different
     * misalignemnt coordiante systems are possible,
     * where one is rotated by another angle (the tangent angle),
     * needs to be added/removed whenever the misalignment is handled.
     * For other elements the tangentangle is simply 0 and the matrix is the
     * identity
     */
    glm::dmat4x4 getTangentAngleRotation();

    glm::dvec4 calcPosition(WorldUserParams prev, glm::dvec4 prev_pos,
                            glm::dmat4x4 prev_or);
    glm::dvec4 calcPosition();
    glm::dmat4x4 calcOrientation(WorldUserParams prev, glm::dmat4x4 prev_or);
    glm::dmat4x4 calcOrientation();

    std::array<double, 6> getMisalignment();
    double getAzimuthalAngle();

  private:
    double m_incidenceAngle;
    double m_exitAngle;
    double m_azimuthalAngle;
    double m_dist;
    std::array<double, 6> m_misalignment;
    /** Only for ellipsoid when the misalignment is defined in the mirror
     * coordinate system and not in the element coordinate system the mirror
     * coordinate system is the element coordinate system rotated by this angle
     * angle = 0 for other elements and if the ellipsoid misalignment is NOT in
     * the mirror coordinate system
     */
    double m_tangentAngle;
};

}  // namespace RAYX
