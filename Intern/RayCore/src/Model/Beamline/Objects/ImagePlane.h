#pragma once
#include "Model/Surface/Quadric.h"
#include "Model/Beamline/OpticalElement.h"

namespace RAYX
{

    class RAYX_API ImagePlane : public OpticalElement {

    public:

        ImagePlane(const char* name, const double distance, const std::shared_ptr<OpticalElement> previous, bool global);
        ImagePlane(const char* name, glm::dvec4 position, glm::dmat4x4 orientation);
        ImagePlane();
        ~ImagePlane();

        static std::shared_ptr<ImagePlane> createFromXML(rapidxml::xml_node<>*);

        double getDistance();

    private:
        double m_distance;
    };
}