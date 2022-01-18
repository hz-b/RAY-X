#include "Ellipsoid.h"

#include "Debug.h"

namespace RAYX {

/**
 * Angles given in degree and stored in rad. Initializes transformation matrices
 * from position and orientation, and parameters for the quadric in super class
 * (surface). stores mirror-specific parameters in this class.
 *
 * @param width             width of the mirror (x-dimension)
 * @param height            height of the mirror (z-dimension)
 * @param azimuthalAngle        azimuthal angle of object (rotation in xy plane
 * with respect to previous element) in rad
 * @param position          position of the element in world coordinates
 * @param orientation       orientation of the element in world coordinates
 * @param grazingIncidence  desired incidence angle of the main ray
 * @param entranceArmLength length of entrance arm
 * @param exitArmLength     length of exit arm
 * @param figRot            figure of rotation (0 = yes, 1 = plane, 2 = no,
 * short half axis C)??
 * @param a_11              a_11 in quadric equation
 * @param slopeError        7 slope error parameters: x-y sagittal (0), y-z
 * meridional (1), thermal distortion: x (2),y (3),z (4), cylindrical bowing
 * amplitude y(5) and radius (6)
 * @param mat               material (See Material.h)
 *
 */
Ellipsoid::Ellipsoid(const char* name,
                     Geometry::GeometricalShape geometricalShape,
                     const double width, const double height,
                     const double azimuthalAngle, glm::dvec4 position,
                     glm::dmat4x4 orientation, const double grazingIncidence,
                     const double entranceArmLength, const double exitArmLength,
                     const int figRot, const double a_11,
                     const std::array<double, 7> slopeError, Material mat)
    : OpticalElement(name, geometricalShape, width, height, azimuthalAngle,
                     position, orientation, slopeError),
      m_incidence(degToRad(grazingIncidence)),
      m_entranceArmLength(entranceArmLength),
      m_exitArmLength(exitArmLength),
      m_a11(a_11) {
    RAYX_LOG << name;
    m_offsetY0 =
        0;  // what is this for? RAYX.FOR: "only !=0 in case of Monocapillary"

    m_figureRotation =
        (figRot == 0 ? FR_YES : (figRot == 1 ? FR_PLANE : FR_A11));
    calcHalfAxes();
    calculateCenterFromHalfAxes(m_incidence);

    // a33, 34, 44
    m_a33 = pow(m_shortHalfAxisB / m_longHalfAxisA, 2);
    m_a34 = m_z0 * m_a33;
    m_a44 = -pow(m_shortHalfAxisB, 2) + pow(m_y0, 2) +
            pow(m_z0 * m_shortHalfAxisB / m_longHalfAxisA, 2);
    m_radius = m_y0;

    double icurv = 1;
    double matd = (double)static_cast<int>(mat);
    setSurface(std::make_unique<Quadric>(
        std::array<double, 4 * 4>{m_a11, 0, 0, 0,         //
                                  icurv, 1, 0, m_radius,  //
                                  0, 0, m_a33, m_a34,     //
                                  7, 0, matd, m_a44}));
    setElementParameters({sin(m_tangentAngle), cos(m_tangentAngle), m_y0,
                          m_z0,        //
                          0, 0, 0, 0,  //
                          0, 0, 0, 0,  //
                          0, 0, 0, 0});
}
// User-defined Parm constructor
Ellipsoid::Ellipsoid(const char* name,
                     Geometry::GeometricalShape geometricalShape,
                     const double width, const double height,
                     const double azimuthalAngle, glm::dvec4 position,
                     const double LongHalfAxisA, const double ShortHalfAxisB,
                     const double DesignAngle, glm::dmat4x4 orientation,
                     const double grazingIncidence,
                     const double entranceArmLength, const double exitArmLength,
                     const int figRot, const double a_11,
                     const std::array<double, 7> slopeError, Material mat)
    : OpticalElement(name, geometricalShape, width, height, azimuthalAngle,
                     position, orientation, slopeError),
      m_incidence(degToRad(grazingIncidence)),
      m_entranceArmLength(entranceArmLength),
      m_exitArmLength(exitArmLength),
      m_a11(a_11),
      m_shortHalfAxisB(ShortHalfAxisB),
      m_longHalfAxisA(LongHalfAxisA),
      m_DesignGrazingAngle(degToRad(DesignAngle)) {
    RAYX_LOG << name << " Manual";
    m_offsetY0 =
        0;  // what is this for? RAY.FOR: "only !=0 in case of Monocapillary"

    m_figureRotation =
        (figRot == 0 ? FR_YES : (figRot == 1 ? FR_PLANE : FR_A11));

    // if design angle not given, take incidenceAngle
    calculateCenterFromHalfAxes(m_DesignGrazingAngle);

    // calculate half axis C
    if (m_figureRotation == FR_YES) {
        m_halfAxisC = m_shortHalfAxisB;  // sqrt(pow(m_shortHalfAxisB, 2) / 1);
                                         // devided by 1??
    } else if (m_figureRotation == FR_PLANE) {
        m_halfAxisC = INFINITY;
    } else {
        m_halfAxisC = sqrt(pow(m_shortHalfAxisB, 2) / m_a11);
    }

    RAYX_LOG << "A= " << m_longHalfAxisA << ", B= " << m_shortHalfAxisB
             << ", C= " << m_halfAxisC;

    // a33, 34, 44
    m_a33 = pow(m_shortHalfAxisB / m_longHalfAxisA, 2);
    m_a34 = m_z0 * m_a33;
    m_a44 = -pow(m_shortHalfAxisB, 2) + pow(m_y0, 2) +
            pow(m_z0 * m_shortHalfAxisB / m_longHalfAxisA, 2);
    m_radius = m_y0;

    double icurv = 1;
    double matd = (double)static_cast<int>(mat);
    setSurface(std::make_unique<Quadric>(
        std::array<double, 4 * 4>{m_a11, 0, 0, 0,         //
                                  icurv, 1, 0, m_radius,  //
                                  0, 0, m_a33,
                                  m_a34,  //
                                  7, 0, matd, m_a44}));
    setElementParameters({sin(m_tangentAngle), cos(m_tangentAngle), m_y0,
                          m_z0,        //
                          0, 0, 0, 0,  //
                          0, 0, 0, 0,  //
                          0, 0, 0, 0});
}

// dstr
Ellipsoid::~Ellipsoid() {}

/*
void Ellipsoid::calcRadius() {
    double theta = m_alpha; // grazing incidence in rad
    m_radius = 2.0/sin(theta) / (1.0 / m_entranceArmLength + 1.0 /
m_exitArmLength);
}*/

void Ellipsoid::calculateCenterFromHalfAxes(double angle) {
    // TODO: is mt = 0 a good default for the case that it'll never be set?
    double mt = 0;  // tangent slope
    if (m_longHalfAxisA > m_shortHalfAxisB) {
        if (angle > 0) {
            m_y0 = -pow(m_shortHalfAxisB, 2) * 1 / tan(angle) /
                   sqrt(pow(m_longHalfAxisA, 2) - pow(m_shortHalfAxisB, 2));
        } else {
            m_y0 = -m_shortHalfAxisB;
        }
    } else {
        m_y0 = 0.0;
    }
    if (m_entranceArmLength > m_exitArmLength && -m_shortHalfAxisB < m_y0) {
        m_z0 = m_longHalfAxisA * sqrt(pow(m_shortHalfAxisB, 2) - pow(m_y0, 2)) /
               m_shortHalfAxisB;
    } else if (m_entranceArmLength < m_exitArmLength &&
               -m_shortHalfAxisB < m_y0) {
        m_z0 = -m_longHalfAxisA *
               sqrt(pow(m_shortHalfAxisB, 2) - pow(m_y0, 2)) / m_shortHalfAxisB;
    } else {
        m_z0 = 0.0;
    }
    if (m_longHalfAxisA > 0.0 && m_y0 < 0.0) {
        mt = -pow(m_shortHalfAxisB / m_longHalfAxisA, 2) * m_z0 / m_y0;
    }
    m_tangentAngle = (atan(mt));
    RAYX_LOG << ", Z0 = " << m_z0 << ", Y0= " << m_y0
             << ", tangentAngle= " << m_tangentAngle;
}

/**
 *  caclulates the half axes, tangent angle and the center of the ellipsoid (z0,
 * y0) from the incidence angle, entrance and exit arm lengths, see ELLPARAM in
 * RAYX.FOR
 */
void Ellipsoid::calcHalfAxes() {
    double theta = m_incidence;  // designGrazingIncidenceAngle always equal to
                                 // alpha (grazingIncidenceAngle)??
    if (theta > PI / 2) {
        theta = PI / 2;
    }
    double a = 0.5 * (m_entranceArmLength + m_exitArmLength);

    double angle = atan(tan(theta) * (m_entranceArmLength - m_exitArmLength) /
                        (m_entranceArmLength + m_exitArmLength));
    m_y0 = m_entranceArmLength * sin(theta - angle);
    double b = a * m_y0 * tan(angle);
    b = 0.25 * pow(m_y0, 4) + pow(b, 2);
    b = sqrt(0.5 * m_y0 * m_y0 + sqrt(b));

    m_z0 = 0;  // center of ellipsoid y0,z0
    if (b != 0) {
        m_z0 = (a / b) * (a / b) * m_y0 * tan(angle);
    }

    // << ellparam in RAY.for to calculate long and short half axis A and B
    m_longHalfAxisA = a;
    m_shortHalfAxisB = b;

    // calculate half axis C
    if (m_figureRotation == FR_YES) {
        m_halfAxisC = sqrt(pow(m_shortHalfAxisB, 2) / 1);  // devided by 1??
    } else if (m_figureRotation == FR_PLANE) {
        m_halfAxisC = INFINITY;
    } else {
        m_halfAxisC = sqrt(pow(m_shortHalfAxisB, 2) / m_a11);
    }
    m_tangentAngle = angle;
    RAYX_LOG << "A= " << m_longHalfAxisA << ", B= " << m_shortHalfAxisB
             << ", C= " << m_halfAxisC
             << ", angle = " << radToDeg(m_tangentAngle);
}

double Ellipsoid::getRadius() const { return m_radius; }

double Ellipsoid::getExitArmLength() const { return m_exitArmLength; }

double Ellipsoid::getEntranceArmLength() const { return m_entranceArmLength; }

double Ellipsoid::getY0() const { return m_y0; }

double Ellipsoid::getZ0() const { return m_z0; }
double Ellipsoid::getIncidenceAngle() const { return m_incidence; }

double Ellipsoid::getShortHalfAxisB() const { return m_shortHalfAxisB; }
double Ellipsoid::getLongHalfAxisA() const { return m_longHalfAxisA; }
double Ellipsoid::getOffsetY0() const { return m_offsetY0; }
double Ellipsoid::getTangentAngle() const { return m_tangentAngle; }
double Ellipsoid::getA34() const { return m_a34; }
double Ellipsoid::getA33() const { return m_a33; }
double Ellipsoid::getA44() const { return m_a44; }
double Ellipsoid::getHalfAxisC() const { return m_halfAxisC; }

// Null if failed
std::shared_ptr<Ellipsoid> Ellipsoid::createFromXML(
    rapidxml::xml_node<>* node, const std::vector<xml::Group>& group_context) {
    const char* name = node->first_attribute("name")->value();

    int gs;
    if (!xml::paramInt(node, "geometricalShape", &gs)) {
        return nullptr;
    }
    Geometry::GeometricalShape geometricalShape =
        static_cast<Geometry::GeometricalShape>(gs);

    double width;
    if (!xml::paramDouble(node, "totalWidth", &width)) {
        return nullptr;
    }

    double height;
    if (!xml::paramDouble(node, "totalLength", &height)) {
        return nullptr;
    }

    double incidenceAngle;
    if (!xml::paramDouble(node, "grazingIncAngle", &incidenceAngle)) {
        return nullptr;
    }

    double mEntrance;
    if (!xml::paramDouble(node, "entranceArmLength", &mEntrance)) {
        return nullptr;
    }

    double mExit;
    if (!xml::paramDouble(node, "exitArmLength", &mExit)) {
        return nullptr;
    }

    double mAzimAngle;
    if (!xml::paramDouble(node, "azimuthalAngle", &mAzimAngle)) {
        return nullptr;
    }
    double m_a11;
    if (!xml::paramDouble(node, "parameter_a11", &m_a11)) {
        return nullptr;
    }
    int m_figRot;
    if (!xml::paramInt(node, "figureRotation", &m_figRot)) {
        return nullptr;
    }

    std::array<double, 7> slopeError;
    if (!xml::paramSlopeError(node, &slopeError)) {
        return nullptr;
    }

    Material mat;
    if (!xml::paramMaterial(node, &mat)) {
        mat = Material::CU;  // default to copper
    }

    // TODO: Are values stored as 0.0 if set to AUTO?[RAY-UI]
    double mDesignGrazing;
    if (!xml::paramDouble(node, "designGrazingIncAngle", &mDesignGrazing)) {
        return nullptr;
    }

    double mlongHalfAxisA;
    if (!xml::paramDouble(node, "longHalfAxisA", &mlongHalfAxisA)) {
        return nullptr;
    }

    double mshortHalfAxisB;
    if (!xml::paramDouble(node, "shortHalfAxisB", &mshortHalfAxisB)) {
        return nullptr;
    }

    // if old ray ui file, need to recalculate position and orientation because
    // those in rml file are wrong. not necessary when our recalculated position
    // and orientation is stored
    double mdistancePreceding;
    if (!xml::paramDouble(node, "distancePreceding", &mdistancePreceding)) {
        return nullptr;
    }

    int mCoordSys;
    if (!xml::paramInt(node, "misalignmentCoordinateSystem", &mCoordSys)) {
        return nullptr;
    }

    std::array<double, 6> mis;
    if (!xml::paramMisalignment(node, &mis)) {
        return nullptr;
    }
    glm::dvec4 position;
    glm::dmat4x4 orientation;

    // do not do this because position and orientation is wrong in RAY-UI
    // files???
    if (!xml::paramPositionAndOrientation(node, group_context, &position,
                                          &orientation)) {
        return nullptr;
    }

    GeometricUserParams g_params =
        GeometricUserParams(incidenceAngle, mEntrance, mExit);

    double tangentAngle =
        g_params.calcTangentAngle(incidenceAngle, mEntrance, mExit, mCoordSys);
    WorldUserParams w_coord = WorldUserParams(
        g_params.getAlpha(), g_params.getBeta(), degToRad(mAzimAngle),
        mdistancePreceding, mis, tangentAngle);
    position = w_coord.calcPosition();
    orientation = w_coord.calcOrientation();

    if ((mDesignGrazing == 0.0) && (mlongHalfAxisA == 0.0) &&
        (mshortHalfAxisB == 0.0)) {  // Auto calculation
        return std::make_shared<Ellipsoid>(
            name, geometricalShape, width, height, degToRad(mAzimAngle),
            position, orientation, incidenceAngle, mEntrance, mExit, m_figRot,
            m_a11, slopeError, mat);
    } else {
        return std::make_shared<Ellipsoid>(
            name, geometricalShape, width, height, degToRad(mAzimAngle),
            position, mlongHalfAxisA, mshortHalfAxisB, mDesignGrazing,
            orientation, incidenceAngle, mEntrance, mExit, m_figRot, m_a11,
            slopeError, mat);
    }
}
}  // namespace RAYX
