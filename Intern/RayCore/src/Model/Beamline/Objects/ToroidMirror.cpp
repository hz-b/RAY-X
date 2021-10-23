#include "ToroidMirror.h"

namespace RAYX
{

    /**
     * Initializes transformation matrices, and parameters for the toroid in super class (optical element).
     * Sets mirror-specific parameters in this class.
     *
     * @param name                      name of the element
     * @param geometricalShape          either rectangular or elliptical
     * @param width                     width of the element
     * @param height                    height of the element
     * @param grazingIncidenceAngle     angle in which the main ray should hit the element, used to calculate the radii, in rad
     * @param position                  position of element in world coordinate system
     * @param orientation               orientation of element in world coordinate system
     * @param mEntrance                 meridional entrance arm length, used to calculate the radii
     * @param mExit                     meridional exit arm length, used to calculate the radii
     * @param sEntrance                 sagittal entrance arm length, used to calculate the radii
     * @param sExit                     sagittal exit arm length, used to calculate the radii
     * @param slopeError                7 slope error parameters: x-y sagittal (0), y-z meridional (1), thermal distortion x (2),y (3),z (4), cylindrical bowing amplitude y(5) and radius (6)
    */
    ToroidMirror::ToroidMirror(const char* name, const int geometricalShape, const double width, const double height, glm::dvec4 position, glm::dmat4x4 orientation, const double incidenceAngle, const double mEntrance, const double mExit, const double sEntrance, const double sExit, const std::vector<double> slopeError)
        : OpticalElement(name, { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 }, geometricalShape, width, height, position, orientation, { 0,0,0, 0,0,0 }, slopeError),
        m_sagittalEntranceArmLength(sEntrance),
        m_sagittalExitArmLength(sExit),
        m_meridionalEntranceArmLength(mEntrance),
        m_meridionalExitArmLength(mExit)
    {
        // TODO(Theresa): maybe move this function outside of this class (same for spheres) because this is derived from user parameters
        calcRadius(incidenceAngle); // calculate the radius

        std::cout << "long Radius: " << m_longRadius << ", short Radius: " << m_shortRadius << std::endl;
        setSurface(std::make_unique<Toroid>(std::vector<double>{m_longRadius, m_shortRadius, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 6, 0, 0, 0}));
        // setSurface(std::make_unique<Toroid>(m_longRadius, m_shortRadius));
    }

    

    ToroidMirror::~ToroidMirror()
    {
    }

    std::shared_ptr<ToroidMirror> ToroidMirror::createFromXML(rapidxml::xml_node<>* node) {
        const char* name = node->first_attribute("name")->value();

        int geometricalShape;
        if (!xml::paramInt(node, "gemetricalShape", &geometricalShape)) { return nullptr; }

        double width;
        if (!xml::paramDouble(node, "totalWidth", &width)) { return nullptr; }

        double height;
        if (!xml::paramDouble(node, "totalLength", &height)) { return nullptr; }

        glm::dvec4 position;
        if (!xml::paramPosition(node, &position)) { return nullptr; }

        glm::dmat4x4 orientation;
        if (!xml::paramOrientation(node, &orientation)) { return nullptr; }

        double incidenceAngle;
        if (!xml::paramDouble(node, "grazingIncAngle", &incidenceAngle)) { return nullptr; }

        double mEntrance;
        if (!xml::paramDouble(node, "entranceArmLengthMer", &mEntrance)) { return nullptr; }

        double mExit;
        if (!xml::paramDouble(node, "exitArmLengthMer", &mExit)) { return nullptr; }

        double sEntrance;
        if (!xml::paramDouble(node, "entranceArmLengthSag", &sEntrance)) { return nullptr; }

        double sExit;
        if (!xml::paramDouble(node, "exitArmLengthSag", &sExit)) { return nullptr; }

        std::vector<double> slopeError;
        if (!xml::paramSlopeError(node, &slopeError)) { return nullptr; }

        return std::make_shared<ToroidMirror>(name, geometricalShape, width, height, position, orientation, incidenceAngle, mEntrance, mExit, sEntrance, sExit, slopeError);
    }


    /**
     * calculate long and short radius from grazing incidence angle and meridional and sagittal entrance and exit arm lengths
     */
    void ToroidMirror::calcRadius(double incidenceAngle) {
        m_longRadius = 2.0 / sin(incidenceAngle) / (1.0 / m_meridionalEntranceArmLength + 1.0 / m_meridionalExitArmLength);

        if (m_meridionalEntranceArmLength == 0.0 || m_meridionalExitArmLength == 0.0 || incidenceAngle == 0.0) {
            m_shortRadius = 0.0;
        }
        else {
            m_shortRadius = 2.0 * sin(incidenceAngle) / (1.0 / m_sagittalEntranceArmLength + 1.0 / m_sagittalExitArmLength);
        }
    }

    double ToroidMirror::getSagittalEntranceArmLength() const {
        return m_sagittalEntranceArmLength;
    }

    double ToroidMirror::getSagittalExitArmLength() const {
        return m_sagittalExitArmLength;
    }

    double ToroidMirror::getMeridionalEntranceArmLength() const {
        return m_meridionalEntranceArmLength;
    }

    double ToroidMirror::getMeridionalExitArmLength() const {
        return m_meridionalExitArmLength;
    }
}