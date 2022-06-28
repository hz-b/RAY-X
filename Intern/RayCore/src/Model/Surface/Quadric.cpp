#include "Quadric.h"

#include <cmath>

#include "Debug.h"

namespace RAYX {
/** for quadric surfaces: 16 parameters a_11,a_12,a_13,a_14,
 * a_21,a_22,a_23,a_24, a_31,a_32,a_33,a_34, a_41,a_42,a_43,a_44!
 * a_21,a_31,a_32,a_41,a_42,a_43 are never used for quadric surfaces because the
 * matrix is symmetrial, can be used for other values
 * @param inputPoints      4x4 Matrix as vector
 */
Quadric::Quadric(const std::array<double, 4*4> inputPoints) {
    m_parameters = inputPoints;
}

Quadric::Quadric() {}  // TODO

Quadric::~Quadric() {}

/**
 * set a new set of parameters a_11 to a_44 for the quadric function
 * order: a_11,a_12,a_13,a_14, a_21,a_22,a_23,a_24, a_31,a_32,a_33,a_34,
 * a_41,a_42,a_43,a_44
 * @param inputPoints   16 entry vector a_11 to a_44
 * @return void
 */
void Quadric::setAnchorPoints(std::array<double, 4*4> inputPoints) {
    m_parameters = inputPoints;
}

/**
 * ENCODING:
 *
 * {a_11,  a_12,     a_13,      a_14,
 *  icurv, a_22,     a_23,      a_24,
 *  0.0,   0.0,      a_33,      a_34,
 *  type,  settings, material,  a_44}
 *
 * @param type = what kind of optical element (mirror, plane grating, spherical
 *grating, toroid mirror, rzp, slit..)
 * @param setting = how to interpret the input of these params. During normal
 *tracing always = 0 but when testing, this parameter defines which test to run
 * @param icurv = whether to take the first or second intersection of a ray with
 *a quadric surface
 * @param a_11, .., a_44 parameters of the quadric equation to find the
 *intersection point. Depend on the element (plane, sphere, ellipsoid,..)
 * @param material = which material the surface has, stored as a number
 *corresponding to the variants of the enum class Material (see Material.h and
 *materials.xmacro)
 **/
std::array<double, 4*4> Quadric::getParams() const {
    RAYX_LOG << "return surface parameters";
    return m_parameters;
}

}  // namespace RAYX
