project(lscm)

cmake_minimum_required(VERSION 3.17)

set (CMAKE_CXX_STANDARD 14)

# setupReconstruction output directories
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# additional CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# OpenMesh
set(OPENMESH_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../OpenMesh/lib)
find_package(OpenMesh REQUIRED)
include_directories(${OPENMESH_INCLUDE_DIRS})

set(ENV{EIGEN3_ROOT} ${CMAKE_CURRENT_SOURCE_DIR}/../eigen)
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

set(CXXOPTS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cxxopts/include)
include_directories(${CXXOPTS_INCLUDE_DIR})

set(CIMG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../CImg)
include_directories(${CIMG_INCLUDE_DIR})
add_compile_definitions(cimg_display=0)

set_property(
    DIRECTORY
    APPEND PROPERTY COMPILE_DEFINITIONS _USE_MATH_DEFINES
)

# collect sources
file(GLOB all_sources src/*/*.c* src/main.cpp)
file(GLOB all_headers src/*/*.h)

# compile and link
add_executable(lscm ${all_sources} ${all_headers})
target_link_libraries(lscm ${OPENMESH_LIBRARIES} ${CMAKE_DL_LIBS})
