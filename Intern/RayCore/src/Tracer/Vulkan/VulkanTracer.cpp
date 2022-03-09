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
    m_bufferSizes.resize(7);
    m_buffers.resize(7);
    m_bufferMemories.resize(7);
    RAYX_LOG << "initialize tracer";
    // Vulkan is initialized
    initVulkan();
    // beamline.resize(0);
}

VulkanTracer::~VulkanTracer() {}

//	This function creates a debug messenger
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
    RAYX_PROFILE_FUNCTION();
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

//	This function destroys the debug messenger
void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator) {
    RAYX_PROFILE_FUNCTION();
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

/* Function is used to start the Vulkan tracer
 */
void VulkanTracer::run() {
    RAYX_PROFILE_FUNCTION();
    const clock_t begin_time = clock();
    RAYX_LOG << "Starting Vulkan Tracer..";
    // Redundant
    // generate the rays used by the tracer
    // generateRays();

    // the sizes of the input and output buffers are set. The buffers need to be
    // the size numberOfRays * size of a Ray * the size of a double (a ray
    // consists of 16 values in double precision, x,y,z for the position and s0,
    // s1, s2, s3; Weight, direction, energy, stokes, pathLEngth, order,
    // lastElement and extraParam.
    // IMPORTANT: The shader size of the buffer needs to be multiples of 32
    // bytes!
    RAYX_LOG << "Setting buffers:";
    // staging buffers need to be at the end of the buffer vector!
    m_bufferSizes[0] = (uint64_t)m_numberOfRays *
                       VULKANTRACER_RAY_DOUBLE_AMOUNT * sizeof(double);
    m_bufferSizes[1] = (uint64_t)m_numberOfRays *
                       VULKANTRACER_RAY_DOUBLE_AMOUNT * sizeof(double);
    m_bufferSizes[2] =
        (VULKANTRACER_QUADRIC_PARAM_DOUBLE_AMOUNT * sizeof(double)) +
        (m_beamlineData.size() * sizeof(double));  // 4 doubles for parameters
    m_bufferSizes[3] =
        std::min((uint64_t)GPU_MAX_STAGING_SIZE,
                 (uint64_t)m_numberOfRays * VULKANTRACER_RAY_DOUBLE_AMOUNT *
                     sizeof(double));  // maximum of 128MB
    m_bufferSizes[4] = (uint64_t)m_numberOfRays * 4 * sizeof(double);
    m_bufferSizes[5] = getMaterialIndexTable()->size() * sizeof(double);
    m_bufferSizes[6] = getMaterialTable()->size() * sizeof(double);
    for (uint32_t i = 0; i < m_bufferSizes.size(); i++) {
        RAYX_LOG << "Buffer [" << i << "] of size : " << m_bufferSizes[i]
                 << " Bytes";
    }

    // initVulkan();
    RAYX_LOG << "Vulkan initilized. Run-time: "
             << float(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << " ms";
    prepareBuffers();

    const clock_t begin_time_getRays = clock();

    mainLoop();

    getRays();

    RAYX_LOG << "Got Rays. Run-time: "
             << float(clock() - begin_time_getRays) / CLOCKS_PER_SEC * 1000
             << " ms";
}

// function for initializing vulkan
void VulkanTracer::initVulkan() {
    RAYX_PROFILE_FUNCTION();
    // a vulkan instance is created
    createInstance();

    setupDebugMessenger();

    // physical device for computation is chosen
    pickPhysicalDevice();

    // a logical device is created
    // it is needed to communicate with the physical device
    createLogicalDevice();

    // create command pool which will be used to submit the staging buffer
    createCommandPool();

    // creates the descriptors used to bind the buffer to shader access points
    // (bindings)
    createDescriptorSetLayout();

    /*prepareBuffers();
    // needs buffer sizes:
    createDescriptorSet();
    RAYX_LOG << "descriptor set created";

    // a compute pipeline needs to be created
    // the descriptors created earlier will be submitted here
    createComputePipeline();
    RAYX_LOG << "compute pipeline created";

    createCommandBuffer();
    RAYX_LOG << "command buffer created";*/
    // creates the descriptors used to bind the buffer to shader access points
    // (bindings)
}

void VulkanTracer::prepareBuffers() {
    // creates buffers to transfer data to and from the shader
    createBuffers();
    const clock_t begin_time_fillBuffer = clock();
    fillRayBuffer();
    RAYX_LOG << "RayBuffer filled, run time: "
             << float(clock() - begin_time_fillBuffer) / CLOCKS_PER_SEC * 1000
             << " ms";
    fillQuadricBuffer();
    fillMaterialBuffer();

    createDescriptorSet();

    // a compute pipeline needs to be created
    // the descriptors created earlier will be submitted here
    createComputePipeline();
    createCommandBuffer();
}

void VulkanTracer::mainLoop() {
    RAYX_PROFILE_FUNCTION();
    const clock_t begin_time = clock();
    runCommandBuffer();
    RAYX_LOG << "CommandBuffer, run time: "
             << float(clock() - begin_time) / CLOCKS_PER_SEC * 1000 << " ms";
}

/** Cleans and deletes the whole tracer instance. Do this only if you do not
 * want to reuse the instance anymore
 * CALL CLEANTRACER FIRST BEFORE CALLING THIS ONE
 */
void VulkanTracer::cleanup() {
    RAYX_PROFILE_FUNCTION();
    vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
    vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
    {
        RAYX_PROFILE_SCOPE("vkDestroyDevice");
        vkDestroyDevice(m_Device, nullptr);
    }
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
    }
    {
        RAYX_PROFILE_SCOPE("vkDestroyInstance");
        vkDestroyInstance(m_Instance, nullptr);
    }
}

/**
 * Use this function if you want to reuse the tracer instance with a new
 * beamline and new rays etc but do not want to initialize everything again
 */
void VulkanTracer::cleanTracer() {
    m_RayList.clean();
    m_beamlineData.clear();
    m_OutputRays.clean();
    vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &m_CommandBuffer);
    vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
    vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
    vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);

    for (uint32_t i = 0; i < m_buffers.size(); i++) {
        vkDestroyBuffer(m_Device, m_buffers[i], nullptr);
        vkFreeMemory(m_Device, m_bufferMemories[i], nullptr);
    }
    vkDestroyShaderModule(m_Device, m_ComputeShaderModule, nullptr);
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
void VulkanTracer::setupDebugMessenger() {
    RAYX_PROFILE_FUNCTION();
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr,
                                     &m_DebugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
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
    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        RAYX_WARN << "(ValidationLayer): " << pCallbackData->pMessage;
    } else if (messageSeverity ==
               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        RAYX_ERR << "(ValidationLayer): " << pCallbackData->pMessage;
    }
    // TODO consider also showing INFO or even VERBOSE messages under some
    // circumstances.

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

// physical device for computation is chosen
void VulkanTracer::pickPhysicalDevice() {
    RAYX_PROFILE_FUNCTION();
    // search for devices
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
    if (deviceCount == 0)
        throw std::runtime_error("failed to find GPUs with Vulkan Support!");

    // create vector of devices
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

    // pick fastest device
    m_PhysicalDevice = VK_NULL_HANDLE;
    int currentRating = -1;

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            int rating = rateDevice(device);
            if (rating > currentRating) {
                m_PhysicalDevice = device;
                currentRating = rating;
            }
        }
    }
    if (m_PhysicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &deviceProperties);
    RAYX_LOG << "Chose GPU: " << deviceProperties.deviceName;
}

// checks if given device is suitable for computation
// can be extended later if more specific features are needed
bool VulkanTracer::isDeviceSuitable(VkPhysicalDevice device) {
    RAYX_PROFILE_FUNCTION();
    // get device properties
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    RAYX_LOG << "Found GPU:" << deviceProperties.deviceName;

    // get device features
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    QueueFamilyIndices indices = findQueueFamilies(device);
    return indices.hasvalue;
}

// Giving scores to each Physical Device
int VulkanTracer::rateDevice(VkPhysicalDevice device) {
    RAYX_PROFILE_FUNCTION();
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    int score = 1;
    // discrete GPUs are usually faster and get a bonus
    // can be extended to choose the best discrete gpu if multiple are
    // available
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        score += 100000;
    score += deviceProperties.limits.maxComputeSharedMemorySize;
    return score;
}

VulkanTracer::QueueFamilyIndices VulkanTracer::findQueueFamilies(
    VkPhysicalDevice device) {
    RAYX_PROFILE_FUNCTION();
    QueueFamilyIndices indices;
    indices.hasvalue = 0;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                             queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            indices.computeFamily = i;
            indices.hasvalue = 1;
        }
        if (indices.isComplete()) break;
        i++;
    }
    m_QueueFamily = indices;
    return indices;
}

// creates a logical device to communicate with the physical device
void VulkanTracer::createLogicalDevice() {
    RAYX_PROFILE_FUNCTION();
    QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice);

    // create info about the device queues
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.computeFamily;
    queueCreateInfo.queueCount = 1;

    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    // create info about the device features
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.shaderFloat64 = true;
    deviceFeatures.shaderInt64 = true;
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;

    auto extensions = getRequiredDeviceExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // enable validation layers if possible
    if (enableValidationLayers) {
        createInfo.enabledLayerCount =
            static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(m_Device, indices.computeFamily, 0, &m_ComputeQueue);
}

// find memory type with desired properties.
uint32_t VulkanTracer::findMemoryType(uint32_t memoryTypeBits,
                                      VkMemoryPropertyFlags properties) {
    RAYX_PROFILE_FUNCTION();
    VkPhysicalDeviceMemoryProperties memoryProperties;

    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memoryProperties);

    /*
    How does this search work?
    See the documentation of VkPhysicalDeviceMemoryProperties for a detailed
    description.
    */
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
        if ((memoryTypeBits & (1 << i)) &&
            ((memoryProperties.memoryTypes[i].propertyFlags & properties) ==
             properties))
            return i;
    }
    return -1;
}

void VulkanTracer::createBuffers() {
    RAYX_PROFILE_FUNCTION();
    // Ray Buffer
    createBuffer(
        m_bufferSizes[0],
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_buffers[0], m_bufferMemories[0]);
    // output Buffer
    createBuffer(
        m_bufferSizes[1],
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_buffers[1], m_bufferMemories[1]);
    // Quadric Buffer
    createBuffer(m_bufferSizes[2], VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                 m_buffers[2], m_bufferMemories[2]);
    // staging buffer for rays
    createBuffer(m_bufferSizes[3],
                 VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                     VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                 m_buffers[3], m_bufferMemories[3]);
    // buffer for xyznull
    createBuffer(m_bufferSizes[4], VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_buffers[4],
                 m_bufferMemories[4]);
    // buffer for material index table
    createBuffer(m_bufferSizes[5], VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                 m_buffers[5], m_bufferMemories[5]);
    // buffer for material table
    createBuffer(m_bufferSizes[6], VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                 m_buffers[6], m_bufferMemories[6]);
    RAYX_LOG << "All buffers created!";
}

void VulkanTracer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                VkMemoryPropertyFlags properties,
                                VkBuffer& buffer,
                                VkDeviceMemory& bufferMemory) {
    RAYX_PROFILE_FUNCTION();
    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = size;
    bufferCreateInfo.usage = usage;  // buffer is used as a storage buffer.
    bufferCreateInfo.sharingMode =
        VK_SHARING_MODE_EXCLUSIVE;  // buffer is exclusive to a single queue
                                    // family at a time.
    VK_CHECK_RESULT(vkCreateBuffer(m_Device, &bufferCreateInfo, NULL, &buffer));
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(m_Device, buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize =
        memoryRequirements.size;  // specify required memory.
    allocateInfo.memoryTypeIndex =
        findMemoryType(memoryRequirements.memoryTypeBits, properties);
    VK_CHECK_RESULT(
        vkAllocateMemory(m_Device, &allocateInfo, NULL,
                         &bufferMemory));  // allocate memory on device.

    // Now associate that allocated memory with the buffer. With that, the
    // buffer is backed by actual memory.
    VK_CHECK_RESULT(vkBindBufferMemory(m_Device, buffer, bufferMemory, 0));
}

void VulkanTracer::fillRayBuffer() {
    RAYX_PROFILE_FUNCTION();
    uint32_t bytesNeeded =
        m_numberOfRays * VULKANTRACER_RAY_DOUBLE_AMOUNT * sizeof(double);
    uint32_t numberOfStagingBuffers =
        std::ceil((double)bytesNeeded /
                  (double)m_bufferSizes[3]);  // bufferSizes[3] = 128MB
    RAYX_LOG << "Number of staging Buffers: " << numberOfStagingBuffers
             << ", (Bytes needed): " << bytesNeeded << " Bytes";
    std::list<std::vector<Ray>>::iterator raySetIterator;
    RAYX_LOG << "Debug Info: rayList.size()= " << m_RayList.size();
    RAYX_LOG << "Staging...";
    raySetIterator = m_RayList.begin();
    size_t vectorsPerStagingBuffer =
        std::floor(GPU_MAX_STAGING_SIZE / RAY_VECTOR_SIZE);
    for (uint32_t i = 0; i < numberOfStagingBuffers - 1; i++) {
        fillStagingBuffer(i, raySetIterator, vectorsPerStagingBuffer);
        std::advance(raySetIterator, vectorsPerStagingBuffer);
        copyToRayBuffer(i * GPU_MAX_STAGING_SIZE, GPU_MAX_STAGING_SIZE);
        RAYX_LOG << "Debug Info: more than 128MB of rays!";
        bytesNeeded = bytesNeeded - GPU_MAX_STAGING_SIZE;
    }

    fillStagingBuffer((numberOfStagingBuffers - 1) * GPU_MAX_STAGING_SIZE,
                      raySetIterator,
                      std::ceil((double)bytesNeeded / RAY_VECTOR_SIZE));

    RAYX_LOG << "Done.";
    copyToRayBuffer((numberOfStagingBuffers - 1) * GPU_MAX_STAGING_SIZE,
                    ((bytesNeeded - 1) % GPU_MAX_STAGING_SIZE) + 1);
}

// the input buffer is filled with the ray data
void VulkanTracer::fillStagingBuffer(
    [[maybe_unused]] uint32_t offset,
    std::list<std::vector<Ray>>::iterator raySetIterator,
    size_t vectorsPerStagingBuffer)  // TODO is it okay that offset is unused?
{
    RAYX_PROFILE_FUNCTION();
    // data is copied to the buffer
    void* data;
    vkMapMemory(m_Device, m_bufferMemories[3], 0, m_bufferSizes[3], 0, &data);

    if ((*raySetIterator).size() > GPU_MAX_STAGING_SIZE) {
        RAYX_ERR << "(*raySetIterator).size() > GPU_MAX_STAGING_SIZE)!";
    }
    vectorsPerStagingBuffer =
        std::min(m_RayList.size(), vectorsPerStagingBuffer);
    RAYX_LOG << "Vectors per StagingBuffer: " << vectorsPerStagingBuffer;
    for (uint32_t i = 0; i < vectorsPerStagingBuffer; i++) {
        memcpy(((char*)data) + i * RAY_VECTOR_SIZE, (*raySetIterator).data(),
               std::min((*raySetIterator).size() *
                            VULKANTRACER_RAY_DOUBLE_AMOUNT * sizeof(double),
                        (size_t)GPU_MAX_STAGING_SIZE));
        raySetIterator++;
    }
    double* temp = (double*)data;
    RAYX_LOG << "Debug Info: value: " << temp[0];
    vkUnmapMemory(m_Device, m_bufferMemories[3]);
    RAYX_LOG << "Vector in StagingBuffer insterted! [RayList→StagingBuffer]";
}

void VulkanTracer::copyToRayBuffer(uint32_t offset,
                                   uint32_t numberOfBytesToCopy) {
    RAYX_PROFILE_FUNCTION();
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.dstOffset = offset;
    copyRegion.size = numberOfBytesToCopy;
    RAYX_LOG << "Copying [Staging→RayBuffer]: offset: " << offset
             << " size: " << numberOfBytesToCopy << " Bytes";
    vkCmdCopyBuffer(commandBuffer, m_buffers[3], m_buffers[0], 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_ComputeQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_ComputeQueue);

    vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
    RAYX_LOG << "Done.";
}
void VulkanTracer::copyToOutputBuffer(uint32_t offset,
                                      uint32_t numberOfBytesToCopy) {
    RAYX_PROFILE_FUNCTION();
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    RAYX_LOG << "Copying [OutputBuffer→StagingBuffer]: offset: " << offset
             << " size: " << numberOfBytesToCopy;
    copyRegion.srcOffset = offset;
    copyRegion.size = numberOfBytesToCopy;
    vkCmdCopyBuffer(commandBuffer, m_buffers[1], m_buffers[3], 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_ComputeQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_ComputeQueue);

    vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &commandBuffer);
}
void VulkanTracer::getRays() {
    RAYX_PROFILE_FUNCTION();
    RAYX_LOG << "Debug Info: m_RayList.size(): " << m_RayList.size();
    // reserve enough data for all the rays
    /*
    data.reserve((uint64_t)numberOfRays * VULKANTRACER_RAY_DOUBLE_AMOUNT);
    */
    uint32_t bytesNeeded =
        m_numberOfRays * VULKANTRACER_RAY_DOUBLE_AMOUNT * sizeof(double);
    uint32_t numberOfStagingBuffers =
        std::ceil((double)bytesNeeded / (double)m_bufferSizes[3]);
    RAYX_LOG << "getRays: Number of staging Buffers: "
             << numberOfStagingBuffers;

    // TODO: ONLY FIRST STAGING BUFFER IS TRANSFERED
    // numberOfStagingBuffers = 1;

    for (uint32_t i = 0; i < numberOfStagingBuffers - 1; i++) {
        std::vector<Ray> data(GPU_MAX_STAGING_SIZE /
                              (RAY_DOUBLE_COUNT * sizeof(double)));
        copyToOutputBuffer(i * GPU_MAX_STAGING_SIZE, GPU_MAX_STAGING_SIZE);
        RAYX_LOG << "Debug Info: more than 128MB of rays";
        void* mappedMemory = NULL;
        // Map the buffer memory, so that we can read from it on the CPU.
        vkMapMemory(m_Device, m_bufferMemories[3], 0, GPU_MAX_STAGING_SIZE, 0,
                    &mappedMemory);
        memcpy(data.data(), mappedMemory, GPU_MAX_STAGING_SIZE);
        data.resize(GPU_MAX_STAGING_SIZE / (RAY_DOUBLE_COUNT * sizeof(double)));
        RAYX_LOG << "getRays: data.size(): " << data.size();
        RAYX_LOG << "getRays: m_outputData.size() before insert: "
                 << m_OutputRays.size();
        m_OutputRays.insertVector(std::move(data));
        RAYX_LOG << "getRays: m_outputData.size() after insert: "
                 << m_OutputRays.size();
        vkUnmapMemory(m_Device, m_bufferMemories[3]);
        bytesNeeded = bytesNeeded - GPU_MAX_STAGING_SIZE;
    }
    RAYX_LOG << "Output Data size: " << m_OutputRays.size();

    copyToOutputBuffer((numberOfStagingBuffers - 1) * GPU_MAX_STAGING_SIZE,
                       ((bytesNeeded - 1) % GPU_MAX_STAGING_SIZE) + 1);

    void* mappedMemory = NULL;
    // Map the buffer memory, so that we can read from it on the CPU.
    vkMapMemory(m_Device, m_bufferMemories[3], 0,
                ((bytesNeeded - 1) % GPU_MAX_STAGING_SIZE) + 1, 0,
                &mappedMemory);
    std::vector<Ray> data((((bytesNeeded - 1) % GPU_MAX_STAGING_SIZE) + 1) /
                          (RAY_DOUBLE_COUNT * sizeof(double)));

    RAYX_LOG << "data size: " << data.size()
             << " bytes needed: " << bytesNeeded;
    memcpy(data.data(), mappedMemory,
           ((bytesNeeded - 1) % GPU_MAX_STAGING_SIZE) + 1);
    data.resize(((((bytesNeeded - 1) % GPU_MAX_STAGING_SIZE) + 1) /
                 (RAY_DOUBLE_COUNT * sizeof(double))));
    RAYX_LOG << "data size: " << data.size();
    m_OutputRays.insertVector(std::move(data));
    vkUnmapMemory(m_Device, m_bufferMemories[3]);
    RAYX_LOG << "Output Data size: " << m_OutputRays.size();
    RAYX_LOG << "Output Data size: "
             << (*(m_OutputRays.begin())).size() * RAY_DOUBLE_COUNT *
                    sizeof(double)
             << " Bytes";
    RAYX_LOG << "Done fetching [StagingBufer→OutputData].";
}

// the quad buffer is filled with the quadric data
void VulkanTracer::fillQuadricBuffer() {
    RAYX_PROFILE_FUNCTION();
    RAYX_LOG << "Filling QuadricBuffer..";
    // data is copied to the buffer
    void* data;
    vkMapMemory(m_Device, m_bufferMemories[2], 0, m_bufferSizes[2], 0, &data);
    memcpy(data, m_beamlineData.data(), m_bufferSizes[2]);
    RAYX_LOG << "Done!";
    vkUnmapMemory(m_Device, m_bufferMemories[2]);
}

void VulkanTracer::fillMaterialBuffer() {
    RAYX_LOG << "Filling MaterialBuffer..";

    // material index buffer
    {
        // data is copied to the buffer
        void* data;
        vkMapMemory(m_Device, m_bufferMemories[5], 0, m_bufferSizes[5], 0,
                    &data);
        memcpy(data, getMaterialIndexTable()->data(), m_bufferSizes[5]);
        vkUnmapMemory(m_Device, m_bufferMemories[5]);
    }

    // material buffer
    {
        void* data;
        vkMapMemory(m_Device, m_bufferMemories[6], 0, m_bufferSizes[6], 0,
                    &data);
        memcpy(data, getMaterialTable()->data(), m_bufferSizes[6]);
        vkUnmapMemory(m_Device, m_bufferMemories[6]);
    }
    RAYX_LOG << "Done!";
}

void VulkanTracer::createDescriptorSetLayout() {
    RAYX_PROFILE_FUNCTION();
    /*
    Here we specify a descriptor set layout. This allows us to bind our
    descriptors to resources in the shader.
    */

    /*
    Here we specify a binding of type VK_DESCRIPTOR_TYPE_STORAGE_BUFFER to
    the binding point 0. This binds to layout(std140, binding = 0) buffer
    ibuf (input) and layout(std140, binding = 1) buffer obuf (output) in the
    compute shader.
    */
    // bindings 0, 1 and 2 are used right now
    VkDescriptorSetLayoutBinding descriptorSetLayoutBinding[] = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
         NULL},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
         NULL},
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
         NULL},
        {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
         NULL},
        {4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
         NULL},
        {5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT,
         NULL}};

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    // 2 bindings are used in this layout
    descriptorSetLayoutCreateInfo.bindingCount = 6;
    descriptorSetLayoutCreateInfo.pBindings =
        descriptorSetLayoutBinding;  // TODO

    // Create the descriptor set layout.
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_Device,
                                                &descriptorSetLayoutCreateInfo,
                                                NULL, &m_DescriptorSetLayout));
}

void VulkanTracer::createDescriptorSet() {
    RAYX_PROFILE_FUNCTION();
    /*
    one descriptor for each buffer
    */
    VkDescriptorPoolSize descriptorPoolSize = {};
    descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorPoolSize.descriptorCount = 6;

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets =
        1;  // we need to allocate one descriptor sets from the pool.
    descriptorPoolCreateInfo.poolSizeCount = 1;
    descriptorPoolCreateInfo.pPoolSizes = &descriptorPoolSize;

    // create descriptor pool.
    VK_CHECK_RESULT(vkCreateDescriptorPool(m_Device, &descriptorPoolCreateInfo,
                                           NULL, &m_DescriptorPool));

    /*
    With the pool allocated, we can now allocate the descriptor set.
    */
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool =
        m_DescriptorPool;  // pool to allocate from.
    descriptorSetAllocateInfo.descriptorSetCount =
        1;  // allocate a single descriptor set.
    descriptorSetAllocateInfo.pSetLayouts = &m_DescriptorSetLayout;

    // allocate descriptor set.
    VK_CHECK_RESULT(vkAllocateDescriptorSets(
        m_Device, &descriptorSetAllocateInfo, &m_DescriptorSet));

    for (uint32_t i = 0; i < m_buffers.size(); i++) {
        if (i == 3) {
            continue;
        }  // the staging buffer is not part of the descriptor set.

        // specify which buffer to use: input buffer
        VkDescriptorBufferInfo descriptorBufferInfo = {};
        descriptorBufferInfo.buffer = m_buffers[i];
        descriptorBufferInfo.offset = 0;
        descriptorBufferInfo.range = m_bufferSizes[i];

        VkWriteDescriptorSet writeDescriptorSet = {};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.pNext = NULL;
        writeDescriptorSet.dstSet =
            m_DescriptorSet;  // write to this descriptor set.
        writeDescriptorSet.dstBinding =
            i - (i >= 4);  // write to the ist binding, or i-1 th binding
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorCount = 1;  // update a single descriptor.
        writeDescriptorSet.descriptorType =
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;  // storage buffer.
        writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;

        // perform the update of the descriptor set.
        vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, NULL);
    }
}

// Read file into array of bytes, and cast to uint32_t*, then return.
// The data has been padded, so that it fits into an array uint32_t.
uint32_t* VulkanTracer::readFile(uint32_t& length, const char* filename) {
    RAYX_PROFILE_FUNCTION();
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Could not find or open file: %s\n", filename);
    }

    // get file size.
    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    long filesizepadded = long(ceil(filesize / 4.0)) * 4;

    // read file contents.
    char* str = DBG_NEW char[filesizepadded];
    fread(str, filesize, sizeof(char), fp);
    fclose(fp);

    // data padding.
    for (int i = filesize; i < filesizepadded; i++) {
        str[i] = 0;
    }

    length = filesizepadded;
    return (uint32_t*)str;
}

void VulkanTracer::createComputePipeline() {
    RAYX_PROFILE_FUNCTION();
    /*
    We create a compute pipeline here.
    */

    /*
    Create a shader module. A shader module basically just encapsulates some
    shader code.
    */
    uint32_t filelength;
    // the code in comp.spv was created by running the command:
    // glslangValidator.exe -V shader.comp
    std::string path = resolvePath(std::string("build/bin/") + SHADERPATH);
    uint32_t* code = readFile(filelength, path.c_str());
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pCode = code;
    createInfo.codeSize = filelength;

    RAYX_LOG << "Create shader module";

    VK_CHECK_RESULT(vkCreateShaderModule(m_Device, &createInfo, NULL,
                                         &m_ComputeShaderModule));
    RAYX_LOG << "Shader module created";
    delete[] code;

    /*
    Now let us actually create the compute pipeline.
    It only consists of a single stage with a compute shader.
    So first we specify the compute shader stage, and it's entry
    point(main).
    */
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {};
    shaderStageCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageCreateInfo.module = m_ComputeShaderModule;
    shaderStageCreateInfo.pName = "main";

    /*
    The pipeline layout allows the pipeline to access descriptor sets.
    So we just specify the descriptor set layout we created earlier.
    */
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout;
    VK_CHECK_RESULT(vkCreatePipelineLayout(m_Device, &pipelineLayoutCreateInfo,
                                           NULL, &m_PipelineLayout));

    VkComputePipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.stage = shaderStageCreateInfo;
    pipelineCreateInfo.layout = m_PipelineLayout;

    /*
    Now, we finally create the compute pipeline.
    */
    VK_CHECK_RESULT(vkCreateComputePipelines(
        m_Device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, NULL, &m_Pipeline));
}

void VulkanTracer::createCommandPool() {
    RAYX_PROFILE_FUNCTION();
    /*
    In order to send commands to the device(GPU),
    we must first record commands into a command buffer.
    To allocate a command buffer, we must first create a command pool. So
    let us do that.
    */
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.flags = 0;
    // the queue family of this command pool. All command buffers allocated
    // from this command pool, must be submitted to queues of this family
    // ONLY.
    commandPoolCreateInfo.queueFamilyIndex = m_QueueFamily.computeFamily;
    VK_CHECK_RESULT(vkCreateCommandPool(m_Device, &commandPoolCreateInfo, NULL,
                                        &m_CommandPool));
}

void VulkanTracer::createCommandBuffer() {
    RAYX_PROFILE_FUNCTION();
    RAYX_LOG << "Creating commandBuffer..";
    /*
    Now allocate a command buffer from the command pool.
    */
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType =
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool =
        m_CommandPool;  // specify the command pool to allocate from.
    // if the command buffer is primary, it can be directly submitted to
    // queues. A secondary buffer has to be called from some primary command
    // buffer, and cannot be directly submitted to a queue. To keep things
    // simple, we use a primary command buffer.
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount =
        1;  // allocate a single command buffer.
    VK_CHECK_RESULT(vkAllocateCommandBuffers(
        m_Device, &commandBufferAllocateInfo,
        &m_CommandBuffer));  // allocate command buffer.

    /*
    Now we shall start recording commands into the newly allocated command
    buffer.
    */
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags =
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  // the buffer is only
                                                      // submitted and used
                                                      // once in this
                                                      // application.
    VK_CHECK_RESULT(vkBeginCommandBuffer(
        m_CommandBuffer, &beginInfo));  // start recording commands.

    /*
    We need to bind a pipeline, AND a descriptor set before we dispatch.
    The validation layer will NOT give warnings if you forget these, so be
    very careful not to forget them.
    */
    vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                      m_Pipeline);
    vkCmdBindDescriptorSets(m_CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, NULL);
    // vkCmdBindDescriptorSets(commandBuffer,
    // VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1,
    // &descriptorSets[1], 0, NULL);
    /*
    Calling vkCmdDispatch basically starts the compute pipeline, and
    executes the compute shader. The number of workgroups is specified in
    the arguments. If you are already familiar with compute shaders from
    OpenGL, this should be nothing new to you.
    */
    RAYX_LOG << "Dispatching commandBuffer...";
    vkCmdDispatch(m_CommandBuffer,
                  (uint32_t)ceil(m_numberOfRays / float(WORKGROUP_SIZE)), 1, 1);

    VK_CHECK_RESULT(
        vkEndCommandBuffer(m_CommandBuffer));  // end recording commands.
}

void VulkanTracer::runCommandBuffer() {
    RAYX_PROFILE_FUNCTION();
    /*
    Now we shall finally submit the recorded command buffer to a queue.
    */

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;  // submit a single command buffer
    submitInfo.pCommandBuffers =
        &m_CommandBuffer;  // the command buffer to submit.

    /*
        We create a fence.
    */
    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0;
    VK_CHECK_RESULT(vkCreateFence(m_Device, &fenceCreateInfo, NULL, &fence));

    /*
    We submit the command buffer on the queue, at the same time giving a
    fence.
    */
    VK_CHECK_RESULT(vkQueueSubmit(m_ComputeQueue, 1, &submitInfo, fence));
    /*
    The command will not have finished executing until the fence is
    signalled. So we wait here. We will directly after this read our buffer
    from the GPU, and we will not be sure that the command has finished
    executing unless we wait for the fence. Hence, we use a fence here.
    */
    VK_CHECK_RESULT(
        vkWaitForFences(m_Device, 1, &fence, VK_TRUE, 1000000000000000));

    vkDestroyFence(m_Device, fence, NULL);
}
void VulkanTracer::setBeamlineParameters(uint32_t inNumberOfBeamlines,
                                         uint32_t inNumberOfQuadricsPerBeamline,
                                         uint32_t inNumberOfRays) {
    RAYX_PROFILE_FUNCTION();
    RAYX_LOG << "Setting Beamline Parameters:";
    RAYX_LOG << "\tNumber of beamlines: " << inNumberOfBeamlines;
    RAYX_LOG << "\tNumber of Quadrics/Beamline: "
             << inNumberOfQuadricsPerBeamline;
    RAYX_LOG << "\tNumber of Rays: " << inNumberOfRays;
    m_numberOfBeamlines = inNumberOfBeamlines;
    m_numberOfQuadricsPerBeamline = inNumberOfQuadricsPerBeamline;
    m_numberOfRays = inNumberOfRays * inNumberOfBeamlines;
    m_numberOfRaysPerBeamline = inNumberOfRays;
    if (m_beamlineData.size() < 4) {
        m_beamlineData.resize(4);
    }
    m_beamlineData[0] = m_numberOfBeamlines;
    m_beamlineData[1] = m_numberOfQuadricsPerBeamline;
    m_beamlineData[2] = m_numberOfRays;
    m_beamlineData[3] = m_numberOfRaysPerBeamline;
}

void VulkanTracer::addRayVector(std::vector<Ray>&& inRayVector) {
    RAYX_PROFILE_FUNCTION();

    RAYX_LOG << "Inserting into rayList. rayList.size() before: "
             << m_RayList.size();
    RAYX_LOG << "Sent size: " << inRayVector.size();
    m_RayList.insertVector(std::move(inRayVector));
    RAYX_LOG << "rayList ray count per vector: "
             << (*(m_RayList.begin())).size();
}

// adds quad to beamline
void VulkanTracer::addArrays(
    const std::array<double, 4 * 4>& surfaceParams,
    const std::array<double, 4 * 4>& inputInMatrix,
    const std::array<double, 4 * 4>& inputOutMatrix,
    const std::array<double, 4 * 4>& objectParameters,
    const std::array<double, 4 * 4>& elementParameters) {
    RAYX_PROFILE_FUNCTION();
    // beamline.resize(beamline.size()+1);

    m_beamlineData.insert(m_beamlineData.end(), surfaceParams.begin(),
                          surfaceParams.end());
    m_beamlineData.insert(m_beamlineData.end(), inputInMatrix.begin(),
                          inputInMatrix.end());
    m_beamlineData.insert(m_beamlineData.end(), inputOutMatrix.begin(),
                          inputOutMatrix.end());
    m_beamlineData.insert(m_beamlineData.end(), objectParameters.begin(),
                          objectParameters.end());
    m_beamlineData.insert(m_beamlineData.end(), elementParameters.begin(),
                          elementParameters.end());

    // Possibility to use utils/movingAppend
}
void VulkanTracer::divideAndSortRays() {
    RAYX_PROFILE_FUNCTION();
    for (auto i = m_RayList.begin(); i != m_RayList.end(); i++) {
    }
}
std::list<std::vector<Ray>>::const_iterator
VulkanTracer::getOutputIteratorBegin() {
    return m_OutputRays.begin();
}
std::list<std::vector<Ray>>::const_iterator
VulkanTracer::getOutputIteratorEnd() {
    return m_OutputRays.end();
}
// is not used anymore
int VulkanTracer::main() {
    RAYX_PROFILE_FUNCTION();
    VulkanTracer app;

    try {
        app.run();
    } catch (const std::exception& e) {
        RAYX_ERR << e.what();
        RAYX_ERR << "VulkanTracer failure!";
        return EXIT_FAILURE;
    }
    RAYX_LOG << "Finished.";
    return EXIT_SUCCESS;
}
}  // namespace RAYX
