#pragma once

#include <memory>
#include <set>
#include <string>
#include <vector>

#include "Core.h"
#include "Model/Beamline/Beamline.h"
#include "Tracer/RayList.h"

namespace RAYX {
/**
 * @brief Abstract Tracer Interface for Tracing "plugins" e.g Vulkan..
 *
 */
class RAYX_API Tracer {
  public:
    Tracer() {}
    virtual ~Tracer() {}
    /**
     * @brief Run the tracing on the Beamline object
     *
     */
    virtual RayList trace(const Beamline&) = 0;
};

}  // namespace RAYX
