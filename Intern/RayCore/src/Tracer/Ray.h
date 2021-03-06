#pragma once
#pragma pack(16)

#include <glm.hpp>

#include "Core.h"

namespace RAYX {
struct RAYX_API Ray {
    glm::dvec3 m_position;
    double m_weight;
    glm::dvec3 m_direction;
    double m_energy;
    glm::dvec4 m_stokes;
    double m_pathLength;
    double m_order;
    double m_lastElement;
    double m_extraParam;

    static Ray makeRayFrom(const glm::dvec3& origin,
                           const glm::dvec3& direction,
                           const glm::dvec4& stokes, const double energy,
                           const double weight) {
        Ray ray;
        ray.m_position = origin;
        ray.m_direction = direction;
        ray.m_stokes = stokes;
        ray.m_energy = energy;
        ray.m_weight = weight;
        ray.m_pathLength = 0.0;
        ray.m_order = 0.0;
        ray.m_lastElement = 0.0;
        ray.m_extraParam = 0.0;
        return ray;
    }

    static Ray makeRayFrom(const glm::dvec3& origin,
                           const glm::dvec3& direction,
                           const glm::dvec4& stokes, const double energy,
                           const double weight, const double pathLength,
                           const double order, const double lastElement,
                           const double extraParam) {
        Ray ray;
        ray.m_position = origin;
        ray.m_direction = direction;
        ray.m_stokes = stokes;
        ray.m_energy = energy;
        ray.m_weight = weight;
        ray.m_pathLength = pathLength;
        ray.m_order = order;
        ray.m_lastElement = lastElement;
        ray.m_extraParam = extraParam;
        return ray;
    }
};
}  // namespace RAYX