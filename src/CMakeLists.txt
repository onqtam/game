ucm_add_flags(${ha_compiler_flags})

include_directories("./")

if(${WITH_PLUGINS})
    add_definitions(-DHA_WITH_PLUGINS)
endif()

# == SHADERS =======================================================================================

if(TOOLCHAIN STREQUAL "msvc")
    FILE(GLOB_RECURSE shaders_vs ${CMAKE_SOURCE_DIR}/data/shaders/raw/*_vs.sc)
    foreach(vs ${shaders_vs})
        add_shader(${vs} VERTEX   OUTPUT ${CMAKE_SOURCE_DIR}/data/shaders GLSL 130 PLATFORMS glsl gles)
    endforeach()
    
    FILE(GLOB_RECURSE shaders_fs ${CMAKE_SOURCE_DIR}/data/shaders/raw/*_fs.sc)
    foreach(fs ${shaders_fs})
        add_shader(${fs} FRAGMENT OUTPUT ${CMAKE_SOURCE_DIR}/data/shaders GLSL 130 PLATFORMS glsl gles)
    endforeach()
    
    #shaderc.exe -f D:/game/data/shaders/raw/cubes/cubes_fs.sc -i D:/game/third_party/bgfx/src -o D:/game/data/shaders/gles/cubes_fs.bin --platform android --type fragment
    
    # add the shaders to the project so they are actually included in the build...
    set(SOURCES ${shaders_vs} ${shaders_fs})
endif()

# == GAME ==========================================================================================

# sadly I need to duplicate these - emscripten (and I think gcc) have trouble building the pch - it does not use the PUBLIC/INTERFACE properties of the targets
include_directories("../third_party/dynamix/include")
include_directories("../third_party/ppk_assert")
include_directories("../third_party/")
include_directories("../third_party/bgfx/include")
include_directories("../third_party/bx/include")
include_directories("../third_party/imgui")
include_directories("../third_party/doctest/doctest")
include_directories("../third_party/tinygizmo")
if(WIN32)
    include_directories("../third_party/nativefiledialog/src/include")
endif()

add_definitions(-DDOCTEST_CONFIG_IMPLEMENTATION_IN_DLL) # imported doctest symbols - test runner in a separate dll

ucm_add_dirs(TO registry_src "core/registry" FILTER_POP 2)
add_library(registry STATIC ${registry_src})
target_link_libraries(registry sajson dynamix)
target_add_fPIC(registry)
target_parse_sources(registry)

# cotire requires the appropriate cpp file for the precompiled header to be the first file in the list of sources
#ucm_add_files(TO SOURCES "precompiled.cpp")
ucm_add_files(TO SOURCES "precompiled.h")
ucm_add_files(TO SOURCES "main.cpp")

ucm_add_dirs(TO SOURCES "core")
ucm_add_dirs(TO SOURCES "core/imgui")
ucm_add_dirs(TO SOURCES "core/serialization")
ucm_add_dirs(TO SOURCES "core/messages")
ucm_add_dirs(TO SOURCES "utils")
ucm_add_dirs(TO SOURCES "utils/base64")
ucm_add_dirs(TO SOURCES "utils/aabb")

# this cannot go inside the game executable because the doctest header is included by the precompiled header in all source files
add_library(doctest_runner SHARED utils/doctest/doctest_runner.cpp utils/doctest/doctest_proxy.h)

# ========= PLUGINS =============

ucm_add_dirs(TO plugin_src "plugins/common" FILTER_POP 1)
ucm_add_dirs(TO plugin_src "plugins/editor" FILTER_POP 1)
add_plugin(NAME plugin ${plugin_src})
ucm_add_dirs(TO plugin_camera "plugins/camera" FILTER_POP 1)
add_plugin(NAME camera ${plugin_camera})

# ========= PLUGINS END =========

add_executable(game "${SOURCES};${PLUGIN_SOURCES}")
add_precompiled_header(game "${CMAKE_CURRENT_SOURCE_DIR}/precompiled.h")

if(NOT MSVC)
    # not sure if this has any effect... should test it
    target_compile_options(game PRIVATE -Wl,--no-undefined)
endif()

target_parse_sources(game)
if(${WITH_PLUGINS})
    set_target_properties(game PROPERTIES ENABLE_EXPORTS ON) # so plugins can link to the executable
    target_compile_definitions(game PRIVATE "HAPI=${ha_symbol_export}")
endif()

target_link_libraries(game bgfx ib-compress)

target_link_libraries(game dynamix)
target_link_libraries(game ppk_assert)
target_link_libraries(game tinygizmo)
target_link_libraries(game imgui)
target_link_libraries(game registry)
target_link_libraries(game doctest_runner)

if(WIN32)
    target_link_libraries(game nativefiledialog)
endif()

if(TOOLCHAIN STREQUAL "js")
    set_target_properties(game PROPERTIES SUFFIX ".html")
else()
    target_include_directories(game PRIVATE ../third_party/glfw/include)
    target_link_libraries(game glew)
    target_link_libraries(game glfw)
    target_link_libraries(game file_watcher)
endif()
