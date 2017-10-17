ucm_add_flags(${ha_third_party_compiler_flags})

# LIBRARY format
#add_subdirectory(format)

#ucm_add_files("format/format.h" "format/format.cc" TO format.sources FILTER_POP 1)
#add_library(format STATIC ${format.sources})

add_library(yama INTERFACE)
target_include_directories(yama INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/yama/include)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/stb)

# LIBRARY sajson
add_library(sajson INTERFACE)
target_include_directories(sajson INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# LIBRARY imgui
ucm_add_dirs("imgui" TO imgui.sources FILTER_POP 1)
add_library(imgui SHARED "${imgui.sources}")
target_include_directories(imgui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
target_compile_definitions(imgui PRIVATE "IMGUI_API=${ha_symbol_export}")
set_target_properties(imgui PROPERTIES FOLDER "3rdparty")
target_add_fPIC(imgui)

# LIBRARY dynamix
ucm_add_dirs("dynamix/include/dynamix" "dynamix/src" RECURSIVE TO dynamix.sources REC FILTER_POP 1)
ucm_add_target(NAME dynamix TYPE SHARED CPP_PER_UNITY 20 SOURCES "${dynamix.sources}")
target_include_directories(dynamix PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/dynamix/include)
set_target_properties(dynamix PROPERTIES FOLDER "3rdparty")
target_add_fPIC(dynamix)

# LIBRARY ppk_assert
ucm_add_dirs("ppk_assert" TO ppk_assert.sources)
add_library(ppk_assert STATIC "${ppk_assert.sources}")
target_include_directories(ppk_assert PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ppk_assert)
set_target_properties(ppk_assert PROPERTIES FOLDER "3rdparty")
target_add_fPIC(ppk_assert)

# LIBRARY tinygizmo
ucm_add_dirs("tinygizmo" TO tinygizmo.sources)
add_library(tinygizmo STATIC "${tinygizmo.sources}")
set_target_properties(tinygizmo PROPERTIES FOLDER "3rdparty")
target_add_fPIC(tinygizmo)

# LIBRARY nativefiledialog
if(WIN32)
    ucm_add_files("nativefiledialog/src/include/nfd.h" TO nativefiledialog.sources FILTER_POP 2)
    ucm_add_files("nativefiledialog/src/nfd_common.c" TO nativefiledialog.sources FILTER_POP 2)
    ucm_add_files("nativefiledialog/src/nfd_win.cpp" TO nativefiledialog.sources FILTER_POP 2)
    # PR for zenity support (to not need gtk3 under linux): https://github.com/mlabbe/nativefiledialog/pull/29
    # or how to gtk3 and cmake:
    # - https://gist.github.com/fracek/3323924
    # - http://francesco-cek.com/cmake-and-gtk-3-the-easy-way/
    #ucm_add_files("nativefiledialog/src/nfd_cocoa.m" TO nativefiledialog.sources FILTER_POP 2)
    #ucm_add_files("nativefiledialog/src/nfd_gtk.cpp" TO nativefiledialog.sources FILTER_POP 2)
    add_library(nativefiledialog STATIC "${nativefiledialog.sources}")
    target_include_directories(nativefiledialog PUBLIC nativefiledialog/src/include)
    set_target_properties(nativefiledialog PROPERTIES FOLDER "3rdparty")
    target_add_fPIC(nativefiledialog)
endif()

# LIBRARY doctest
set(DOCTEST_WITH_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(doctest)

if(NOT TOOLCHAIN STREQUAL "js")
    # LIBRARY glew
    ucm_add_dirs("${GLEW_DIR}/include/" TO glew.sources RECURSIVE FILTER_POP 3)
    ucm_add_files("${GLEW_DIR}/src/glew.c" TO glew.sources FILTER_POP 2)
    add_library(glew SHARED ${glew.sources})
    target_compile_options(glew PRIVATE -DGLEW_BUILD)
    if(UNIX)
        target_link_libraries(glew GL)
    else()
        target_link_libraries(glew opengl32)
    endif()
    set_target_properties(glew PROPERTIES FOLDER "3rdparty")

    # LIBRARY glfw
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

    add_subdirectory(glfw)
    set_target_properties(glfw PROPERTIES FOLDER "3rdparty")

    # LIBRARY file_watcher
    ucm_add_dirs("FileWatcher" TO file_watcher.sources)
    ucm_add_target(NAME file_watcher TYPE STATIC SOURCES "${file_watcher.sources}")
    set_target_properties(file_watcher PROPERTIES FOLDER "3rdparty")
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
