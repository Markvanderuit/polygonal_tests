cmake_minimum_required(VERSION 3.22)

# Specify VCPKG toolchain path, CMAKE module path
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake" CACHE STRING "CMake module path")

# Specify project name, languages
project(PolygonalTests LANGUAGES CXX)
option(PRG_ENABLE_ASSERTIONS "Enable assertions inside build" ON)

# Enable all modules in /cmake
include(add_targets)
include(dependency)
include(spirv)

# Specify output directories as /bin, /lib, /out, and tell MSVC to stuff it
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
if(MSVC)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL     ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL     ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
endif()

# Include the smalL_gl package in /third_party
option(gl_enable_exceptions  "" ${PRG_ENABLE_ASSERTIONS})
option(gl_enable_tracy       "" FALSE)
add_subdirectory(third_party/small_gl)

# Include the packages in vcpkg or system
find_package(Eigen3        CONFIG REQUIRED)
find_package(fmt           CONFIG REQUIRED)
find_package(imgui         CONFIG REQUIRED)
find_package(imguizmo      CONFIG REQUIRED)
find_package(meshoptimizer CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenMP        REQUIRED)
find_package(Qhull         CONFIG REQUIRED)

# Add target metameric_shaders; compiles and copies glsl to spirv 
# from /shaders to /bin/shaders
prg_add_shader_target(shaders 
  ${CMAKE_CURRENT_SOURCE_DIR}/shaders
  ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders
)

# Setup core library
prg_add_library(core "")
target_link_libraries(core 
  PUBLIC Eigen3::Eigen
         fmt::fmt-header-only
         meshoptimizer::meshoptimizer
         Qhull::qhullcpp Qhull::qhull_r
         OpenMP::OpenMP_CXX
         nlohmann_json::nlohmann_json
         small_gl
         imgui::imgui 
         imguizmo::imguizmo
)

# Setup mean value coordinate executable
add_executable(mean_value_coordinates src/app/mean_value_coordinates.cpp)
add_dependencies(mean_value_coordinates shaders)
target_compile_features(mean_value_coordinates PRIVATE cxx_std_23)
target_link_libraries(mean_value_coordinates   PRIVATE core)
