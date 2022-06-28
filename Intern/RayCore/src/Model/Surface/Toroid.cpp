#include "Toroid.h"

#include <cmath>

#include "Debug.h"

namespace RAYX {
/**
 * @param inputPoints           16 entry vector that contains the parameters for
 * a toroidal surface
 */
Toroid::Toroid(const std::array<double, 4*4> inputPoints) {
    m_parameters = inputPoints;
}

/**
 * @param longRadius            long radius of the toroidal shape
 * @param shortRadius           short radius of the toroidal shape
 * @param elementType           6=toroid mirror, 4=toroid rzp
 */
Toroid::Toroid(double longRadius, double shortRadius, double elementType,
               Material mat) {
    double matd = (double)static_cast<int>(mat);
    m_parameters = std::array<double, 4*4>{longRadius,  shortRadius, 0,    0,  //
                                       0,           0,           0,    0,  //
                                       0,           0,           0,    0,  //
                                       elementType, 0,           matd, 0};
    m_longRadius = longRadius;
    m_shortRadius = shortRadius;
}

Toroid::Toroid() {}

Toroid::~Toroid() {}

/**
 * sets the parameters of the toroidal surface to the given 16 entry vector
 * @param inputPoints   16 entry vector
 * @return void
 */
void Toroid::setParameters(const std::array<double, 4*4> inputPoints) {
    m_parameters = inputPoints;
    m_longRadius = inputPoints[0];
    m_shortRadius = inputPoints[1];
}

/**
 * ENCODING:
 *
 * {longRadius,  shortRadius,   0.0,       0.0,
 *  0.0,         0.0,           0.0,       0.0,
 *  0.0,         0.0,           0.0,       0.0,
 *  type,        settings,      material,  0.0}
 *
 * @param type = what kind of optical element (mirror, plane grating, spherical
 *grating, toroid mirror, rzp, slit..)
 * @param setting = how to interpret the input of
 *these params. During normal tracing always = 0 but when testing, this
 *parameter defines which test to run
 * @param material = which material the surface has, stored as a number
 *corresponding to the variants of the enum class Material (see Material.h and
 *materials.xmacro)
 *
 **/
std::array<double, 4*4> Toroid::getParams() const {
    RAYX_LOG << "Return surface points";
    return m_parameters;
}

}  // namespace RAYX
