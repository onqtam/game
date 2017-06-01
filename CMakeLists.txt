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

set(BGFX_BUILD_TOOLS    OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL        OFF CACHE BOOL "" FORCE)
set(BGFX_CUSTOM_TARGETS ON  CACHE BOOL "" FORCE)
set(BGFX_USE_OVR        OFF CACHE BOOL "" FORCE)
set(BGFX_AMALGAMATED    OFF CACHE BOOL "" FORCE)
set(BX_AMALGAMATED      OFF CACHE BOOL "" FORCE)

set(BGFX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/bgfx" CACHE STRING "Location of bgfx.")
set(BX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/bx" CACHE STRING "Location of bx.")
set(BIMG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/bimg" CACHE STRING "Location of bimg.")

if(NOT TOOLCHAIN STREQUAL "js")
#    set(BGFX_BUILD_TOOLS    ON)
#    set(BGFX_BUILD_EXAMPLES ON)
endif()

# overwrite these after the inclusion of ucm
set(UCM_UNITY_BUILD OFF)
set(UCM_NO_COTIRE_FOLDER ON)

option(WITH_PLUGINS "Compile plugins as separate .dll files instead of merging them with the executable" ON)
#set(WITH_PLUGINS ON CACHE BOOL "" FORCE)

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

if(TOOLCHAIN STREQUAL "js")
    include_directories("emscripten/emscripten/1.37.9/system/include")
    set(WITH_PLUGINS OFF)
    ucm_add_flags(-s TOTAL_MEMORY=335544320) # 320 MB - must be a multiple of 16 MB
    ucm_add_flags(--emrun)
    if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        #ucm_add_flags(-s SAFE_HEAP=1)
        ucm_add_flags(-s DEMANGLE_SUPPORT=1 -s ASSERTIONS=2 -s NO_EXIT_RUNTIME=1)
    endif()
    
    add_definitions(-DDOCTEST_CONFIG_NO_POSIX_SIGNALS) # sigaltstack missing in js environment

    # add this because dynamix uses it
    include_directories("${EMSCRIPTEN_ROOT}/system/lib/libcxxabi/include")
endif()

if(${WITH_PLUGINS})
    add_definitions(-DHARDLY_WITH_PLUGINS)
endif()

#===================================================================================================
#== RECURSE THROUGH CMAKE ==========================================================================
#===================================================================================================

add_subdirectory(third_party)
add_subdirectory(src)













