add_subdirectory(tools)

ucm_add_flags(${ha_compiler_flags})

include_directories("./")

if(${WITH_PLUGINS})
    add_definitions(-DHA_WITH_PLUGINS)
endif()

# == GAME ==========================================================================================

# sadly I need to duplicate these - emscripten (and I think gcc) have trouble building the pch - it does not use the PUBLIC/INTERFACE properties of the targets
include_directories("../third_party/dynamix/include")
include_directories("../third_party/ppk_assert")
include_directories("../third_party/")
include_directories("../third_party/imgui")
include_directories("../third_party/doctest/doctest")
include_directories("../third_party/tinygizmo")
if(WIN32)
    include_directories("../third_party/nativefiledialog/src/include")
endif()

add_definitions(-DDOCTEST_CONFIG_IMPLEMENTATION_IN_DLL) # imported doctest symbols - test runner in a separate dll

ucm_add_dirs(TO registry_src "core/registry" FILTER_POP 2)
# added here so the custom_command is executed on it and it gets parsed before compiling the registry
list(APPEND registry_src core/Object.h)
add_library(registry STATIC ${registry_src})
target_link_libraries(registry sajson dynamix)
target_add_fPIC(registry)
target_parse_sources(registry)

ucm_add_files(TO SOURCES "precompiled.h")
ucm_add_files(TO SOURCES "main.cpp")

ucm_add_dirs(TO SOURCES "core")
ucm_add_dirs(TO SOURCES "core/imgui")
ucm_add_dirs(TO SOURCES "core/serialization")
ucm_add_dirs(TO SOURCES "core/messages")
ucm_add_dirs(TO SOURCES "utils")
ucm_add_dirs(TO SOURCES "utils/base64")
ucm_add_dirs(TO SOURCES "utils/aabb")
ucm_add_dirs(TO SOURCES "core/rendering")

message(${SOURCES})

if(MSVC)
    list(APPEND SOURCES 
        rcrl/rcrl.h
        rcrl/rcrl.cpp
        rcrl/rcrl_parser.h
        rcrl/rcrl_parser.cpp
        rcrl/rcrl_for_plugin.h
    )
endif()

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

target_link_libraries(game dynamix)
target_link_libraries(game ppk_assert)
target_link_libraries(game tinygizmo)
target_link_libraries(game imgui)
target_link_libraries(game registry)
target_link_libraries(game doctest_runner)
target_link_libraries(game ImGuiColorTextEdit)
target_link_libraries(game tiny-process-library)

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
