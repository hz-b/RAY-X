cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(RayCore)

SET(SOURCE
    src/Application.cpp
    src/utils.cpp
    src/Model/Beamline/Beamline.cpp
    src/Model/Beamline/BeamlineObject.cpp
    src/Model/Beamline/OpticalElement.cpp
    src/Model/Beamline/LightSource.cpp
    src/Model/Beamline/EnergyDistribution.cpp
    src/Model/Beamline/BeamlineObjectFactory.cpp
    src/Model/Beamline/Objects/ImagePlane.cpp
    src/Model/Beamline/Objects/MatrixSource.cpp
    src/Model/Beamline/Objects/RandomRays.cpp
    src/Model/Beamline/Objects/PointSource.cpp
    src/Model/Beamline/Objects/Ellipsoid.cpp
    src/Model/Beamline/Objects/ReflectionZonePlate.cpp
    src/Model/Beamline/Objects/Slit.cpp
    src/Model/Beamline/Objects/SphereGrating.cpp
    src/Model/Beamline/Objects/PlaneGrating.cpp
    src/Model/Beamline/Objects/PlaneMirror.cpp
    src/Model/Beamline/Objects/PointSource.cpp
    src/Model/Beamline/Objects/SphereMirror.cpp
    src/Model/Beamline/Objects/ToroidMirror.cpp
    src/Model/Beamline/Objects/Cylinder.cpp
    src/Data/Exporter.cpp
    src/Data/Importer.cpp
    src/Data/xml.cpp
    src/Data/DatFile.cpp
    src/Presenter/Presenter.cpp
    src/Presenter/SimulationEnv.cpp
    src/Model/Surface/Quadric.cpp
    src/Model/Surface/Toroid.cpp
    src/Model/Surface/Surface.cpp
    src/Model/Geometry/Geometry.cpp
    src/Tracer/RayList.cpp
    src/Tracer/TracerInterface.cpp
    src/Tracer/Writer/CSVWriter.cpp
    src/Tracer/Writer/H5Writer.cpp
    src/Tracer/TracerInterface.cpp
    src/Tracer/Vulkan/VulkanTracer.cpp
    src/Tracer/Vulkan/Material.cpp
    src/Tracer/Vulkan/PalikTable.cpp
    src/UserParameter/WorldUserParams.cpp
    src/UserParameter/GeometricUserParams.cpp
)

SET(HEADERS
    src/Application.h
    src/utils.h
    src/Model/Beamline/Beamline.h
    src/Model/Beamline/BeamlineObject.h
    src/Model/Beamline/OpticalElement.h
    src/Model/Beamline/LightSource.h
    src/Model/Beamline/EnergyDistribution.h
    src/Model/Beamline/BeamlineObjectFactory.h
    src/Model/Beamline/Objects/ImagePlane.h
    src/Model/Beamline/Objects/MatrixSource.h
    src/Model/Beamline/Objects/RandomRays.h
    src/Model/Beamline/Objects/PointSource.h
    src/Model/Beamline/Objects/Ellipsoid.h
    src/Model/Beamline/Objects/ReflectionZonePlate.h
    src/Model/Beamline/Objects/Slit.h
    src/Model/Beamline/Objects/SphereGrating.h
    src/Model/Beamline/Objects/PlaneGrating.h
    src/Model/Beamline/Objects/PlaneMirror.h
    src/Model/Beamline/Objects/PointSource.h
    src/Model/Beamline/Objects/SphereMirror.h
    src/Model/Beamline/Objects/ToroidMirror.h
    src/Model/Beamline/Objects/Cylinder.h
    src/Data/Exporter.h
    src/Data/Importer.h
    src/Data/xml.h
    src/Data/DatFile.h
    src/Presenter/Presenter.h
    src/Presenter/SimulationEnv.h
    src/Model/Surface/Quadric.h
    src/Model/Surface/Toroid.h
    src/Model/Surface/Surface.h
    src/Model/Geometry/Geometry.h
    src/Tracer/RayList.h
    src/Tracer/Ray.h
    src/Tracer/TracerInterface.h
    src/Tracer/Writer/Writer.hpp
    src/Tracer/Writer/CSVWriter.hpp
    src/Tracer/Writer/H5Writer.hpp
    src/Tracer/Vulkan/VulkanTracer.h
    src/Tracer/Vulkan/Material.h
    src/Tracer/Vulkan/PalikTable.h
    src/UserParameter/WorldUserParams.h
    src/UserParameter/GeometricUserParams.h
)

add_library(${PROJECT_NAME} SHARED ${SOURCE} ${HEADERS} )

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(COMPILE_PLATFORM RAYX_PLATFORM_GCC)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(COMPILE_PLATFORM RAYX_PLATFORM_WINDOWS)
else() 
    set(COMPILE_PLATFORM UNKNOWN)
endif()

target_compile_definitions(${PROJECT_NAME} 
    PRIVATE RAYX_BUILD_DLL
    PUBLIC ${COMPILE_PLATFORM}
)

find_package(Vulkan REQUIRED)
find_package(Python3 COMPONENTS Interpreter Development NumPy REQUIRED)
target_link_libraries(${PROJECT_NAME} 
    PUBLIC Vulkan::Vulkan
    PUBLIC Python3::NumPy
    PUBLIC Python3::Python
    PUBLIC Python3::Module
)

target_include_directories(${PROJECT_NAME}
    SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC ${RayCore_SOURCE_DIR}/../../Extern/glm/glm/
    PUBLIC ${RayCore_SOURCE_DIR}/../../Extern/rapidxml-1.13/
    PUBLIC ${RayCore_SOURCE_DIR}/../../Extern/HighFive/include/
    PUBLIC ${RayCore_SOURCE_DIR}/../../Extern/matplotlib-cpp/
)

add_dependencies(${PROJECT_NAME} COMPILE_SHADER)

# Compile Shaders
set(COMPILED_SHADER ${CMAKE_BINARY_DIR}/bin/comp.spv)
set(COMPILED_SHADER_FAKE ${CMAKE_BINARY_DIR}/bin/___comp.spv) # this exists so file cannot be found -> always execute command
# OS specific
if (WIN32)
    # The following code is used to always compile the shader.
    # This is most likely not an optimal solution, but it will work
    # until we find a better one.
    add_custom_command(
        OUTPUT 
            ${COMPILED_SHADER}
            ${COMPILED_SHADER_FAKE}
        COMMAND "$ENV{VULKAN_SDK}/Bin/glslc.exe"
        ARGS ${RayCore_SOURCE_DIR}/src/Tracer/Vulkan/shader.comp -o ${COMPILED_SHADER}
    )
elseif (UNIX)
    add_custom_command(
        OUTPUT 
            ${COMPILED_SHADER}
            ${COMPILED_SHADER_FAKE}
        COMMAND "glslc"
        ARGS ${RayCore_SOURCE_DIR}/src/Tracer/Vulkan/shader.comp -o ${COMPILED_SHADER}
    )
    # set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
endif ()

add_custom_target(COMPILE_SHADER ALL
    DEPENDS ${COMPILED_SHADER}
)
