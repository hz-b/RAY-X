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
#define VK_CHECK_RESULT(f)                           \
    {                                                \
        VkResult res = (f);                          \
        if (res != VK_SUCCESS) {                     \
            RAYX_ERR << "Fatal : VkResult fail!";    \
        }                                            \
    }
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator);

const int WORKGROUP_SIZE = 32;

class RAYX_API VulkanTracer {
  public:
    VulkanTracer();
    ~VulkanTracer();
    void run();
    // void addRay(double xpos, double ypos, double zpos, double xdir, double
    // ydir, double zdir, double weight); void addRay(double* location);
    // cleans and destroys the whole tracer instance
    // CALL CLEANTRACER BEFORE CALLING THIS ONE
    void cleanup();
    // empties raylist, Beamline and output data, etc. so that the tracer
    // instance can be used again
    void cleanTracer();

    void getRays();
    void addRayVector(std::vector<Ray>&& inRayVector);
    void addArrays(const std::array<double, 4*4>& surfaceParams,
                    const std::array<double, 4*4>& inputInMatrix,
                    const std::array<double, 4*4>& inputOutMatrix,
                    const std::array<double, 4*4>& objectParameters,
                    const std::array<double, 4*4>& elementParameters);
    void setBeamlineParameters(uint32_t inNumberOfBeamlines,
                               uint32_t inNumberOfQuadricsPerBeamline,
                               uint32_t inNumberOfRays);

    std::list<std::vector<Ray>>::const_iterator getOutputIteratorBegin();
    std::list<std::vector<Ray>>::const_iterator getOutputIteratorEnd();

    // getter
    const RayList& getRayList() { return m_RayList; }

  private:
    // Member structs:
    /* not contiguous in memory, shouldn't be used
    struct Quadric{
        Quadric() : points(16), inMatrix(16), outMatrix(16) {}
        Quadric(std::vector<double> inQuadric, std::vector<double>
    inputInMatrix, std::vector<double> inputOutMatrix){ assert(inQuadric.size()
    == 16 && inputInMatrix.size() == 16 && inputOutMatrix.size() == 16); points
    = inQuadric; inMatrix = inputInMatrix; outMatrix = inputOutMatrix;
        }
        std::vector<double> points;
        std::vector<double> inMatrix;
        std::vector<double> outMatrix;
    };
    */

    struct QueueFamilyIndices {
        uint32_t computeFamily;
        bool hasvalue;

        bool isComplete() { return hasvalue; }
    };

    // Member variables:
    VkInstance m_Instance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device;
    std::vector<uint64_t> m_bufferSizes;
    std::vector<VkBuffer> m_buffers;
    std::vector<VkDeviceMemory> m_bufferMemories;
    VkPipeline m_Pipeline;
    VkPipelineLayout m_PipelineLayout;
    VkShaderModule m_ComputeShaderModule;
    VkCommandPool m_CommandPool;
    VkCommandBuffer m_CommandBuffer;
    VkDescriptorPool m_DescriptorPool;
    VkDescriptorSet m_DescriptorSet;
    VkDescriptorSetLayout m_DescriptorSetLayout;
    VkQueue m_ComputeQueue;
    uint32_t m_QueueFamilyIndex;
    QueueFamilyIndices m_QueueFamily;

    // Ray-related vars:
    uint32_t m_numberOfBeamlines;
    uint32_t m_numberOfQuadricsPerBeamline;
    uint32_t m_numberOfRays;
    uint32_t m_numberOfRaysPerBeamline;
    RayList m_RayList;
    std::vector<double> m_beamlineData;
    RayList m_OutputRays;

    // Member functions:
    // Vulkan
    void initVulkan();
    void prepareBuffers();
    void mainLoop();
    void createInstance();
    void populateDebugMessengerCreateInfo(
        VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();
    std::vector<const char*> getRequiredExtensions();
    std::vector<const char*> getRequiredDeviceExtensions();
    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                  void* pUserData);
    bool checkValidationLayerSupport();
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    int rateDevice(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    void createLogicalDevice();
    uint32_t findMemoryType(uint32_t memoryTypeBits,
                            VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties, VkBuffer& buffer,
                      VkDeviceMemory& bufferMemory);
    void createBuffers();
    void fillRayBuffer();
    void fillStagingBuffer(uint32_t offset,
                           std::list<std::vector<Ray>>::iterator raySetIterator,
                           size_t vectorsPerStagingBuffer);
    void createDescriptorSetLayout();
    void createDescriptorSet();
    void createCommandPool();
    void createComputePipeline();
    void createCommandBuffer();
    void runCommandBuffer();

    // Ray-related funcs:
    void divideAndSortRays();
    void fillQuadricBuffer();
    void fillMaterialBuffer();
    void copyToRayBuffer(uint32_t offset, uint32_t numberOfBytesToCopy);
    void copyToOutputBuffer(uint32_t offset, uint32_t numberOfBytesToCopy);

    // Utils
    uint32_t* readFile(uint32_t& length, const char* filename);

    int main();
};
}  // namespace RAYX
