ucm_add_flags(${third_party_compiler_flags})

# LIBRARY format
#add_subdirectory(format)

#ucm_add_files("format/format.h" "format/format.cc" TO format.sources TRIM_FILTER 1)
#add_library(format STATIC ${format.sources})

# LIBRARY glm
#ucm_add_dirs("glm/glm" TO glm.sources REC TRIM_FILTER 2)
#ucm_remove_files("glm/glm/detail/glm.cpp" "glm/glm/detail/dummy.cpp" TO glm.sources)

#add_library(glm STATIC ${glm.sources})
#set_target_properties(glm PROPERTIES LINKER_LANGUAGE CXX)

# LIBRARY dynamix
ucm_add_dirs("dynamix/include/dynamix" "dynamix/src" RECURSIVE TO dynamix.sources REC FILTER_POP 1)
ucm_add_target(NAME dynamix TYPE SHARED CPP_PER_UNITY 20 SOURCES "${dynamix.sources}")
set_target_properties(dynamix PROPERTIES FOLDER "third_party")
add_fPIC_to_target(dynamix)

# LIBRARY ppk_assert
ucm_add_dirs("ppk_assert" TO ppk_assert.sources)
ucm_add_target(NAME ppk_assert TYPE STATIC SOURCES "${ppk_assert.sources}")
set_target_properties(ppk_assert PROPERTIES FOLDER "third_party")
add_fPIC_to_target(ppk_assert)

add_subdirectory(bgfx.cmake)
include(bgfx.cmake/cmake/3rdparty/ib-compress.cmake)
include(bgfx.cmake/cmake/3rdparty/ocornut-imgui.cmake)

if(NOT TOOLCHAIN STREQUAL "js")
    # LIBRARY glfw
    add_subdirectory(glfw)
    set_target_properties(glfw PROPERTIES FOLDER "third_party")
    
    # LIBRARY file_watcher
    ucm_add_dirs("FileWatcher" TO file_watcher.sources)
    ucm_add_target(NAME file_watcher TYPE STATIC SOURCES "${file_watcher.sources}")
    set_target_properties(file_watcher PROPERTIES FOLDER "third_party")
endif()

## LIBRARY assimp
#add_subdirectory(assimp)

## LIBRARY recast
#include_directories("recastnavigation/DebugUtils/Include")
#include_directories("recastnavigation/Detour/Include")
#include_directories("recastnavigation/DetourCrowd/Include")
#include_directories("recastnavigation/DetourTileCache/Include")
#include_directories("recastnavigation/Recast/Include")
#add_dir_rec_2(recast.sources "recastnavigation/DebugUtils" 1)
#add_dir_rec_2(recast.sources "recastnavigation/Detour" 1)
#add_dir_rec_2(recast.sources "recastnavigation/DetourCrowd" 1)
#add_dir_rec_2(recast.sources "recastnavigation/DetourTileCache" 1)
#add_dir_rec_2(recast.sources "recastnavigation/Recast" 1)
#add_library(recast STATIC ${recast.sources})

# LIBRARY freetype
# setting this to off because freetype does not export symbols under windows
#set(BUILD_SHARED_LIBS OFF)
#add_subdirectory(${FREETYPE_DIR})
#if(UNIX)
#    set_target_properties(freetype PROPERTIES COMPILE_FLAGS "-fPIC")
#endif()
