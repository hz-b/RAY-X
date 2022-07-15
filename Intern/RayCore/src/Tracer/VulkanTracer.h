#pragma once

#include <Material/Material.h>

#include <cstdlib>
#include <iostream>
#include <map>
#include <optional>
#include <stdexcept>

#include "Core.h"
#include "Tracer/RayList.h"
#include "Tracer/Tracer.h"
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
#define VULKANTRACER_DEBUG_ENTRY_DOUBLE_AMOUNT 16

namespace RAYX {

// set debug generation information
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

// Used for validating return values of Vulkan API calls.
#define VK_CHECK_RESULT(f)                                               \
    {                                                                    \
        VkResult res = (f);                                              \
        if (res != VK_SUCCESS) {                                         \
            RAYX_WARN << "Fatal : VkResult fail!";                       \
            RAYX_ERR << "Error code: " << res                            \
                     << ", look up at "                                  \
                        "https://www.khronos.org/registry/vulkan/specs/" \
                        "1.3-extensions/man/html/VkResult.html";         \
        }                                                                \
    }
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator);

const int WORKGROUP_SIZE = 32;

class RAYX_API VulkanTracer : public Tracer {
  public:
    VulkanTracer();
    ~VulkanTracer();

    RayList trace(const Beamline&) override;
    bool runTest(int testSettings) override;

  private:
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
    void getDebugBuffer();
    void addRayVector(std::vector<Ray>&& inRayVector);
    void addArrays(const std::array<double, 4 * 4>& surfaceParams,
                   const std::array<double, 4 * 4>& inputInMatrix,
                   const std::array<double, 4 * 4>& inputOutMatrix,
                   const std::array<double, 4 * 4>& objectParameters,
                   const std::array<double, 4 * 4>& elementParameters);
    void setBeamlineParameters(uint32_t inNumberOfBeamlines,
                               uint32_t inNumberOfQuadricsPerBeamline,
                               uint32_t inNumberOfRays);

    // getter
    std::list<std::vector<Ray>>::const_iterator getOutputIteratorBegin();
    std::list<std::vector<Ray>>::const_iterator getOutputIteratorEnd();

    // https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_uniform_buffer_object.txt
    // stf140 align rules (Stick to only 1 matrix for simplicity)
    struct _debugBuf_t {
        glm::dmat4x4 _dMat;  // Set to identiy matrix in shader.
    };
    std::vector<_debugBuf_t>::const_iterator getDebugIteratorBegin();
    std::vector<_debugBuf_t>::const_iterator getDebugIteratorEnd();

    const RayList& getRayList() { return m_RayList; }
    const auto getDebugList() { return m_debugBufList; }

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
    _debugBuf_t m_debug;
    struct Compute {  // Possibilty to add CommandPool, Pipeline etc.. here
        std::vector<uint64_t> m_BufferSizes;
        std::vector<VkBuffer> m_Buffers;
        std::vector<VkDeviceMemory> m_BufferMemories;
    } m_compute;

    struct Graphics {
        // Placeholder for Graphic rendering (For later steps)
    } m_graphics;

    struct Staging {
        std::vector<uint64_t> m_BufferSizes;
        std::vector<VkBuffer> m_Buffers;
        std::vector<VkDeviceMemory> m_BufferMemories;
    } m_staging;

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
    RayList m_OutputRays;
    std::vector<double> m_beamlineData;
    std::vector<_debugBuf_t> m_debugBufList;

    // Material tables
    MaterialTables m_MaterialTables;

	// this currently contains two doubles:
	// - testSettings
	// - return value success, per default it's 0 meaning failure.
	std::vector<double> m_testBufferData;

    struct Settings {
        bool m_isDebug;
        uint32_t m_buffersCount;
        uint32_t m_computeBuffersCount;
        uint32_t m_stagingBuffersCount;
    } m_settings;

    // Member functions:
    // Vulkan
    void prepareVulkan();
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
    void setSettings();
    bool isDebug() const;
    uint32_t getNumberOfBuffers() const;

    // Ray-related funcs:
    void divideAndSortRays();
    void fillQuadricBuffer();
    void fillMaterialBuffer();
    void fillTestBuffer();
    void copyToRayBuffer(uint32_t offset, uint32_t numberOfBytesToCopy);
    void copyToOutputBuffer(uint32_t offset, uint32_t numberOfBytesToCopy);
    void copyFromDebugBuffer(uint32_t offset, uint32_t numberOfBytesToCopy);

    // Utils
    uint32_t* readFile(uint32_t& length, const char* filename);

    int main();
};
}  // namespace RAYX
