#include "VulkanTracer.h"

#include <chrono>
#include <cmath>

#include "Debug.h"
#include "Debug/Instrumentor.h"
#include "Material.h"
#include "PathResolver.h"

#ifdef RAYX_PLATFORM_WINDOWS
#include "GFSDK_Aftermath.h"
#endif

#define SHADERPATH "comp.spv"
#ifndef SHADERPATH
#define SHADERPATH "comp_inter.spv"
#endif
#ifndef SHADERPATH
#define SHADERPATH "comp_dyn.spv"
#endif
#ifndef SHADERPATH
//#define SHADERPATH "comp_dyn_inter.spv"
#endif
#ifndef SHADERPATH
#define SHADERPATH "comp_dyn_multi.spv"
#endif
#ifndef SHADERPATH
#define SHADERPATH "comp_new_all.spv"
#endif

// Memory leak detection in debug mode
#ifdef RAYX_PLATFORM_WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#ifndef NDEBUG
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#else
#define DBG_NEW new
#endif

/**
 * buffers[0] = ray buffer
 * buffers[1] = output buffer
 * buffers[2] = quadric buffer
 * buffers[3] = staging buffer
 * buffers[4] = xyznull buffer
 * buffers[5] = material index table
 * buffers[6] = material table
 **/

namespace RAYX {
VulkanTracer::VulkanTracer() {
    RAYX_LOG << "initialize tracer";
    // Vulkan is initialized
    initVulkan();
    // beamline.resize(0);
}

VulkanTracer::~VulkanTracer() {}

// function for initializing vulkan
void VulkanTracer::initVulkan() {
    RAYX_PROFILE_FUNCTION();
    // a vulkan instance is created
    createInstance();
}

void VulkanTracer::createInstance() {
    RAYX_PROFILE_FUNCTION();
    // validation layers are used for debugging
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error(
            "validation layers requested, but not available!");
    }

    // Add description for instance
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VulkanTracer";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 2, 154);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 2, 154);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // pointer to description with layer count
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledLayerCount = 0;

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    if (enableValidationLayers) {
        createInfo.enabledLayerCount =
            static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext =
            (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    // create instance
    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
    if (result != VK_SUCCESS)
        throw std::runtime_error("failed to create instance!");
}

void VulkanTracer::populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    RAYX_PROFILE_FUNCTION();
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

std::vector<const char*> VulkanTracer::getRequiredExtensions() {
    RAYX_PROFILE_FUNCTION();
    std::vector<const char*> extensions;

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

// Currently returns a  std::vector<const char*>
// Can be further enhanced for propoer Window output with glfw.
std::vector<const char*> VulkanTracer::getRequiredDeviceExtensions() {
    RAYX_PROFILE_FUNCTION();
    std::vector<const char*> extensions;

    // if (enableValidationLayers)
    // {
    // 	extensions.push_back("VK_EXT_descriptor_indexing");
    // }

    return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanTracer::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    [[maybe_unused]] void* pUserData) {
    RAYX_PROFILE_FUNCTION();
    // Only show Warnings or higher severity bits
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        RAYX_ERR << "(ValidationLayer!): " << pCallbackData->pMessage;
    }
    return VK_FALSE;  // Should return False
}

// Checks if all validation layers are supported.
bool VulkanTracer::checkValidationLayerSupport() {
    RAYX_PROFILE_FUNCTION();
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    // check all validation layers
    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) return false;
    }
    return true;
}

}  // namespace RAYX