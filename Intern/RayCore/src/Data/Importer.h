#pragma once

#include <memory>
#include <vector>

#include "Core.h"
#include "Model/Beamline/Beamline.h"

namespace RAYX {

const enum BeamlineElementType {
    CYLINDER,
    ELLIPSOID,
    IMAGE_PLANE,
    MATRIX_SOURCE,
    PLANE_GRATING,
    PLANE_MIRROR,
    POINT_SOURCE,
    RANDOM_RAY,
    RZP,
    SLIT,
    SPHERE_GRATING,
    SPHERE_MIRROR,
    TOROID_MIRROR,
};

// ------RAY-UI related. (Used for backwards compatibility to legacy Sequential
// tracing)
// TODO(OS): Place somewhere else
class RAYX_API BeamlineHierarchy {
  public:
    BeamlineHierarchy();
    ~BeamlineHierarchy();

    std::vector<std::shared_ptr<AbstractBeamlineElement>> getBeamlineElements()
        const;
    void printBeamlineHierarchy();
    void addAbstractBeamlineOject(double incidence, double entranceArmLength,
                                  double exitArmLength, int mCoordSys,
                                  double mAzimAngle, std::array<double, 6> mis,
                                  double distancePreceding,
                                  BeamlineElementType objectType);

  private:
    std::vector<std::shared_ptr<AbstractBeamlineElement>> m_AbstractElements;
};
class RAYX_API AbstractBeamlineElement : public BeamlineObject {
  public:
    AbstractBeamlineElement(double distancePreceding, glm::vec4 position,
                            glm::dmat4x4 orientation,
                            BeamlineElementType objectType);
    ~AbstractBeamlineElement();
    double getDistancePreceding() const;
    glm::dvec4 getPosition() const;
    glm::dmat4x4 getOrientation() const;
    BeamlineElementType getElementType() const;

  private:
    double m_distancePreceding;
    glm::vec4 m_postion;
    glm::dmat4x4 m_orientation;
    BeamlineElementType m_type;
};

class RAYX_API Importer {
  public:
    Importer();
    ~Importer();

    static Beamline importBeamline(const char* filename);

  private:
    std::vector<std::unique_ptr<BeamlineHierarchy>> m_lab;
};

}  // namespace RAYX