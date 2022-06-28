#include "BeamlineObject.h"

#include <cmath>

namespace RAYX {
BeamlineObject::BeamlineObject(const char* name) : m_name(name) {}
BeamlineObject::BeamlineObject() {}
BeamlineObject::~BeamlineObject() {}

const std::string& BeamlineObject::getName() const { return m_name; }

}  // namespace RAYX
