#pragma once

#include <cmath>

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

enum class GratingMount { Deviation, Incidence };
enum class ImageType { Point2Point, Astigmatic2Astigmatic };

class RAYX_API GeometricUserParams {
  public:
    // constructor for plane and sphere mirrors
    GeometricUserParams(double incidenceAngle);
    // constructor for gratings
    GeometricUserParams(GratingMount mount, double deviation,
                        double normalIncidence, double lineDensity,
                        double designEnergy, int orderOfDiffraction);
    // constructor for rzp
    GeometricUserParams(GratingMount mount, ImageType imageType,
                        double deviationAngle, double grazingIncidence,
                        double grazingExitAngle, double sourceEnergy,
                        double designEnergy, double orderOfDiffraction,
                        double designOrderOfDiffraction,
                        double designAlphaAngle, double designBetaAngle,
                        double mEntrance, double mExit, double sEntrance,
                        double sExit);
    // constructor for EllipsoidMirror
    GeometricUserParams(double incidence, double entranceArmLength,
                        double exitArmLength);
    GeometricUserParams();
    ~GeometricUserParams();

    void focus(double angle, double designEnergy, double lineDensity,
               double orderOfDiffraction);
    double calcDz00(ImageType imageType, double designWavelength,
                    double designAlphaAngle, double designBetaAngle,
                    double designOrderOfDiffraction, double sEntrance,
                    double sExit, double mEntrance, double mExit);
    double calcFresnelZOffset(double designAlphaAngle, double designBetaAngle,
                              double sEntrance, double sExit);
    double rzpLineDensityDZ(ImageType imageType, glm::dvec3 intersection,
                            glm::dvec3 normal, double designWavelength,
                            double designAlphaAngle, double designBetaAngle,
                            double designOrderOfDiffraction, double sEntrance,
                            double sExit, double mEntrance, double mExit);
    double calcTangentAngle(double incidence, double entranceArmLength,
                            double exitArmLength, int coordSys);

    void calcMirrorRadius(double entranceArmLength, double exitArmLength);
    void calcGratingRadius(GratingMount mount, double deviation,
                           double entranceArmLength, double exitArmLength);
    void calcTorusRadius(double incidenceAngle, double sEntrance, double sExit,
                         double mEntrance, double mExit);

    double getAlpha();
    double getBeta();
    double getRadius();
    double getShortRadius();

  private:
    double m_alpha;
    double m_beta;
    double m_radius;       // only for spheres and toroid (long radius)
    double m_shortRadius;  // only for toroid
};
}  // namespace RAYX