#pragma once

#include <filesystem>
#include <memory>

#include "Core.h"
#include "Model/Beamline/Beamline.h"

namespace RAYX {

RAYX_API Beamline importBeamline(std::filesystem::path);

}  // namespace RAYX