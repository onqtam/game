cmake_minimum_required(VERSION 3.0.0)

project(All)

cmake_policy(SET CMP0017 OLD)
cmake_policy(SET CMP0051 NEW)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake/cmake_modules")

set(COTIRE_PCH_MEMORY_SCALING_FACTOR 500)
include(third_party/ucm/cmake/ucm.cmake)
include(scripts/cmake/compiler_flags.cmake)
include(scripts/cmake/utils.cmake)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# overwrite these after the inclusion of ucm
set(UCM_UNITY_BUILD OFF)
set(UCM_NO_COTIRE_FOLDER ON)

option(WITH_PLUGINS "Compile plugins as separate .dll files instead of merging them with the executable" ON)

set(WITH_PLUGINS ON CACHE BOOL "" FORCE)

#===================================================================================================
#== OUTPUT DIRS ====================================================================================
#===================================================================================================

set(bin_dir ${TOOLCHAIN})
if(${CMAKE_GENERATOR} STREQUAL "Ninja")
    set(bin_dir "ninja")
endif()

set(OUTPUT_DIR ${PROJECT_BINARY_DIR}/../../bin/${bin_dir}/${CMAKE_BUILD_TYPE})
if(MSVC AND NOT ${CMAKE_GENERATOR} STREQUAL "Ninja")
    set(OUTPUT_DIR ${PROJECT_BINARY_DIR}/../../bin/${bin_dir})
endif()

# for the generic no-config case
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})

#===================================================================================================
#== INCLUDE PATHS ==================================================================================
#===================================================================================================

set(BOOST_DIR "boost_1_64_0")
set(GLEW_DIR "glew-2.0.0")

# this is for find_package() calls
set(BOOST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/${BOOST_DIR}" CACHE PATH "Boost path")

include_directories("third_party")
#include_directories("third_party/stb")
include_directories("third_party/ppk_assert")
include_directories("third_party/dynamix/include")
include_directories("third_party/glm")
#include_directories("third_party/format")
include_directories("third_party/${BOOST_DIR}")
include_directories("third_party/doctest")

if(NOT TOOLCHAIN STREQUAL "js")
    include_directories("third_party/glfw/include")
    include_directories("third_party/${GLEW_DIR}/include")
    
    # use -DGLEW_STATIC for static link with glew
    add_definitions(-DGLEW_BUILD)
    if(UNIX)
        set(GLEW_LINK_LIBRARIES GL)
    else()
        set(GLEW_LINK_LIBRARIES opengl32)
    endif()
else()
    include_directories("emscripten/emscripten/1.37.9/system/include")
    set(WITH_PLUGINS OFF)
    
    ucm_add_flags(-s DEMANGLE_SUPPORT=1 -s ASSERTIONS=2)
endif()

if(${WITH_PLUGINS})
    add_definitions(-DHARDLY_WITH_PLUGINS)
endif()

#===================================================================================================
#== RECURSE THROUGH CMAKE ==========================================================================
#===================================================================================================

add_subdirectory(third_party)
add_subdirectory(src)













