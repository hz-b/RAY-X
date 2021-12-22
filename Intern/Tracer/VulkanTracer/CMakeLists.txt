cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

project(VulkanTracer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC" )
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fPIC" )


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
        ARGS ${VulkanTracer_SOURCE_DIR}/src/shader.comp -o ${COMPILED_SHADER}
    )
elseif (UNIX)
    add_custom_command(
        OUTPUT 
            ${COMPILED_SHADER}
            ${COMPILED_SHADER_FAKE}
        COMMAND "glslc"
        ARGS ${VulkanTracer_SOURCE_DIR}/src/shader.comp -o ${COMPILED_SHADER}
    )
    # set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
endif ()

add_custom_target(COMPILE_SHADER ALL
    DEPENDS ${COMPILED_SHADER}
)

find_package(Vulkan REQUIRED FATAL_ERROR)
if (WIN32)
    #find_package(NsightAftermath REQUIRED FATAL_ERROR)
    #get_filename_component(NsightAftermath_LIBRARY_DIR ${NsightAftermath_LIBRARY} DIRECTORY)
endif ()

add_library(${PROJECT_NAME} 
    STATIC 
        src/Ray.cpp 
        src/Ray.h 
        src/RayList.h
        src/RayList.cpp
        src/PalikTable.h
        src/PalikTable.cpp
        src/Material.h
        src/Material.cpp
        src/VulkanTracer.cpp
        src/VulkanTracer.h
)

target_include_directories(${PROJECT_NAME}  
    PUBLIC $ENV{VULKAN_SDK}/include
    PUBLIC ${VulkanTracer_SOURCE_DIR}/src/
)

target_link_libraries(${PROJECT_NAME}
    Vulkan::Vulkan
)

if (WIN32)
    target_link_libraries(${PROJECT_NAME}
        #NsightAftermath::NsightAftermath
    )
endif ()

add_dependencies(${PROJECT_NAME} COMPILE_SHADER)
