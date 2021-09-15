#pragma once

#include "Beamline/Beamline.h"
#include "Core.h"

#include <string>
#include <vector>
#include <set>
#include <memory>

class VulkanTracer;

namespace RAYX
{
    class Ray;

    class RAYX_API TracerInterface
    {
    public:
        enum m_dataType { RayType, QuadricType };

        TracerInterface();
        ~TracerInterface();
        void addLightSource(std::shared_ptr<LightSource> newSource);
        void generateRays(VulkanTracer& tracer, std::shared_ptr<LightSource> source);
        void addOpticalElementToTracer(VulkanTracer& tracer, std::shared_ptr<OpticalElement> element);
        void writeToFile(const std::vector<double>& outputRays, std::ofstream& file, int index) const;


        bool run(double translationXerror, double translationYerror, double translationZerror);
    private:
        // TODO(Jannis): remove (interfaces like this shouldn't hold important data)
        std::vector<std::shared_ptr<LightSource>> m_LightSources;
        Beamline m_Beamline;
    };
} // namespace RAYX