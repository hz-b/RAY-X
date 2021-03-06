cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(RayCore)

SET(SOURCE
    src/PathResolver.cpp
    src/Debug.cpp
    src/utils.cpp
    src/Data/Exporter.cpp
    src/Data/Importer.cpp
    src/Data/xml.cpp
    src/Data/DatFile.cpp
    src/Model/Beamline/Beamline.cpp
    src/Model/Beamline/OpticalElement.cpp
    src/Model/Beamline/LightSource.cpp
    src/Model/Beamline/EnergyDistribution.cpp
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
    src/Model/Beamline/Objects/Cone.cpp
    src/Model/Surface/Quadric.cpp
    src/Model/Surface/Toroid.cpp
    src/Model/Surface/Surface.cpp
    src/Material/Material.cpp
    src/Material/NffTable.cpp
    src/Material/PalikTable.cpp
    src/Tracer/RayList.cpp
    src/Tracer/VulkanTracer.cpp
    src/Tracer/CpuTracer.cpp
    src/Writer/CSVWriter.cpp
    src/Writer/H5Writer.cpp
    src/UserParameter/WorldUserParams.cpp
    src/UserParameter/GeometricUserParams.cpp
)

SET(HEADERS
    src/PathResolver.h
    src/utils.h
    src/Model/Beamline/Beamline.h
    src/Model/Beamline/OpticalElement.h
    src/Model/Beamline/LightSource.h
    src/Model/Beamline/EnergyDistribution.h
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
    src/Model/Surface/Quadric.h
    src/Model/Surface/Toroid.h
    src/Model/Surface/Surface.h
    src/Tracer/RayList.h
    src/Tracer/Ray.h
    src/Tracer/Tracer.h
    src/Tracer/CpuTracer.h
    src/Writer/Writer.hpp
    src/Writer/CSVWriter.hpp
    src/Writer/H5Writer.hpp
    src/Tracer/VulkanTracer.h
    src/Material/Material.h
    src/Material/NffTable.h
    src/Material/PalikTable.h
    src/UserParameter/WorldUserParams.h
    src/UserParameter/GeometricUserParams.h
)

add_library(${PROJECT_NAME} SHARED ${SOURCE} ${HEADERS} )

target_precompile_headers(${PROJECT_NAME} PRIVATE raycorePCH.h)
set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(COMPILE_PLATFORM RAYX_PLATFORM_GCC)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(COMPILE_PLATFORM RAYX_PLATFORM_MSVC)
else() 
    set(COMPILE_PLATFORM UNKNOWN)
endif()

target_compile_definitions(${PROJECT_NAME}
    PRIVATE RAYX_BUILD_DLL
    PUBLIC ${COMPILE_PLATFORM}
        $<$<CONFIG:Debug>:RAYX_DEBUG_MODE>
        $<$<CONFIG:RelWithDebInfo>:RAYX_DEBUG_MODE>
)

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME}
    PUBLIC Vulkan::Vulkan
)

if (NOT "$ENV{CI}" STREQUAL "YES")
if (UNIX)
    target_include_directories(${PROJECT_NAME} PUBLIC /usr/include/hdf5/serial)
    target_link_directories(${PROJECT_NAME} PUBLIC /usr/lib/x86_64-linux-gnu/hdf5/serial)
    target_link_libraries(${PROJECT_NAME} PUBLIC hdf5)
else()
    find_package(HDF5 REQUIRED)
    target_link_libraries(${PROJECT_NAME}
    PUBLIC HDF5::HDF5
    )
endif()
endif()

target_include_directories(${PROJECT_NAME}
    SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC ${RayCore_SOURCE_DIR}/../../Extern/glm/glm/
    PUBLIC ${RayCore_SOURCE_DIR}/../../Extern/rapidxml-1.13/
    PUBLIC ${RayCore_SOURCE_DIR}/../../Extern/HighFive/include/
)

add_dependencies(${PROJECT_NAME} COMPILE_SHADER)

# If debug or release with debug symbols
if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    message(STATUS "Setting glslc args...")
    set(GLSLC_ARGS "${GLSLC_ARGS} -DRAYX_PLATFORM_WINDOWS")
endif()

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
        ARGS ${RayCore_SOURCE_DIR}/src/Tracer/shader/main-glsl.comp -o ${COMPILED_SHADER}
    )
elseif (UNIX)
    add_custom_command(
        OUTPUT
            ${COMPILED_SHADER}
            ${COMPILED_SHADER_FAKE}
        COMMAND "glslc"
        ARGS ${RayCore_SOURCE_DIR}/src/Tracer/shader/main-glsl.comp -o ${COMPILED_SHADER}
    )
    # set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
endif ()

add_custom_target(COMPILE_SHADER ALL
    DEPENDS ${COMPILED_SHADER}
)
