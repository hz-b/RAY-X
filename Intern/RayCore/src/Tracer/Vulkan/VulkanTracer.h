#pragma once

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>

#include "Core.h"
#include "PalikTable.h"
#include "Tracer/RayList.h"
#include "vulkan/vulkan.hpp"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// Vulkan to Ray #defines
#define VULKANTRACER_RAY_DOUBLE_AMOUNT 16
#define VULKANTRACER_QUADRIC_DOUBLE_AMOUNT 112  // 7* dmat4 (16)
#define VULKANTRACER_QUADRIC_PARAM_DOUBLE_AMOUNT 4
#define GPU_MAX_STAGING_SIZE 134217728  // 128MB
#define RAY_VECTOR_SIZE 16777216

namespace RAYX {

// set debug generation information
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

// Used for validating return values of Vulkan API calls.
#define VK_CHECK_RESULT(f)                        \
    {                                             \
        VkResult res = (f);                       \
        if (res != VK_SUCCESS) {                  \
            RAYX_ERR << "Fatal : VkResult fail!"; \
        }                                         \
    }
const int WORKGROUP_SIZE = 32;

class RAYX_API VulkanTracer {
  public:
    VulkanTracer();
    ~VulkanTracer();

    void populateDebugMessengerCreateInfo(
        VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                  void* pUserData);

  private:

    // Member variables:
    VkInstance m_Instance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;

    void initVulkan();
    void createInstance();
    std::vector<const char*> getRequiredExtensions();
    std::vector<const char*> getRequiredDeviceExtensions();
    bool checkValidationLayerSupport();
};
}  // namespace RAYX
