#include "TracerInterface.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "Debug.h"
#include "Debug/Instrumentor.h"
#include "Model/Beamline/LightSource.h"
#include "Model/Beamline/OpticalElement.h"
#include "Ray.h"
#include "Writer/Writer.hpp"

namespace RAYX {
TracerInterface::TracerInterface() : m_numElements(0), m_numRays(0) {
    RAYX_D_LOG << "Creating TracerInterface...";
}

TracerInterface::TracerInterface(int numElements, int numRays)
    : m_numElements(numElements), m_numRays(numRays) {
    RAYX_D_LOG << "Creating TracerInterface with " << numElements
               << " elements and " << numRays << " rays...";
}

TracerInterface::~TracerInterface() {
    RAYX_D_LOG << "Deleting TracerInterface...";
}

/** Generates rays from light source into the tracer.
 *
 *  @param tracer   reference to tracer
 *  @param source   shared pointer to light source used for ray generation
 */
void TracerInterface::generateRays(std::shared_ptr<LightSource> source) {
    // only one Source for now
    if (!source) return;
    RAYX_D_LOG << "add rays";
    m_RayTracer.addRayVector(source->getRays());
}

void TracerInterface::setBeamlineParameters() {
    m_RayTracer.setBeamlineParameters(1, m_numElements, m_numRays);
}

void TracerInterface::addOpticalElementToTracer(
    std::shared_ptr<OpticalElement> element) {
    m_RayTracer.addArrays(element->getSurfaceParams(), element->getInMatrix(),
                          element->getOutMatrix(),
                          element->getObjectParameters(),
                          element->getElementParameters());
}

bool TracerInterface::run() {
    RAYX_PROFILE_FUNCTION();

    m_RayTracer.run();  // run tracer
    RAYX_D_LOG << "Run succeeded!";

    // transform in to usable data
    auto doubleVecSize = RAY_MAX_ELEMENTS_IN_VECTOR * RAY_DOUBLE_COUNT;
    std::vector<double> doubleVec(doubleVecSize);
    size_t index = 0;

    std::unique_ptr<Writer> w =  // TODO(rudi): maybe allow CSVWriter to also be
                                 // used in non-CI builds
#ifdef CI
        std::make_unique<CSVWriter>();
#else
        std::make_unique<H5Writer>();
#endif

    // get rays from tracer
    for (auto outputRayIterator = m_RayTracer.getOutputIteratorBegin(),
              outputIteratorEnd = m_RayTracer.getOutputIteratorEnd();
         outputRayIterator != outputIteratorEnd; outputRayIterator++) {
        RAYX_D_LOG << "(*outputRayIterator).size(): "
                   << (*outputRayIterator).size();

        memcpy(doubleVec.data(), (*outputRayIterator).data(),
               (*outputRayIterator).size() * VULKANTRACER_RAY_DOUBLE_AMOUNT *
                   sizeof(double));
        doubleVec.resize((*outputRayIterator).size() *
                         VULKANTRACER_RAY_DOUBLE_AMOUNT);

        RAYX_D_LOG << "sample ray: " << doubleVec[0] << ", " << doubleVec[1]
                   << ", " << doubleVec[2] << ", " << doubleVec[3] << ", "
                   << doubleVec[4] << ", " << doubleVec[5] << ", "
                   << doubleVec[6] << ", energy: " << doubleVec[7]
                   << ", stokes 0: " << doubleVec[8];

        w->appendRays(doubleVec, index);
        index = index + (*outputRayIterator).size();
    }

    // clean up tracer to avoid memory leaks
    m_RayTracer.cleanTracer();
    m_RayTracer.cleanup();
    // intentionally not RAYX_DEBUG()
    RAYX_LOG << "Done.";
    return true;
}
}  // namespace RAYX
// namespace RAYX
