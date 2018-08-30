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

# this cannot go inside the game executable because the doctest header is included by the precompiled header in all source files
add_library(doctest_runner SHARED utils/doctest/doctest_runner.cpp utils/doctest/doctest_proxy.h)

# ========= PLUGINS =============

ucm_add_dirs(TO plugin_src "plugins/common" FILTER_POP 1)
ucm_add_dirs(TO plugin_src "plugins/editor" FILTER_POP 1)
add_plugin(NAME plugin ${plugin_src})
ucm_add_dirs(TO plugin_camera "plugins/camera" FILTER_POP 1)
add_plugin(NAME camera ${plugin_camera})

# ========= PLUGINS END =========

if(MSVC AND ${WITH_PLUGINS})
    # add the sources to the game project
    ucm_add_dirs(TO SOURCES "rcrl")
endif()

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

# ========= REPL =============

if(MSVC AND ${WITH_PLUGINS})
    # the file that will 
    set(plugin_file ${PROJECT_BINARY_DIR}/plugin.cpp)
    # touch the file so it exists
    file(WRITE ${plugin_file} "")
    
    # defines needed for RCRL integration
    target_compile_definitions(game PRIVATE "RCRL_PLUGIN_FILE=\"${plugin_file}\"")
    target_compile_definitions(game PRIVATE "RCRL_PLUGIN_NAME=\"plugin\"")
    target_compile_definitions(game PRIVATE "RCRL_BUILD_FOLDER=\"${PROJECT_BINARY_DIR}\"")
    target_compile_definitions(game PRIVATE "RCRL_BIN_FOLDER=\"$<TARGET_FILE_DIR:game>/\"")
    target_compile_definitions(game PRIVATE "RCRL_EXTENSION=\"${CMAKE_SHARED_LIBRARY_SUFFIX}\"")
    if(${CMAKE_GENERATOR} MATCHES "Visual Studio" OR ${CMAKE_GENERATOR} MATCHES "Xcode")
        target_compile_definitions(game PRIVATE "RCRL_CONFIG=\"$<CONFIG>\"")
    endif()
    
    # the plugin target that will be recompiled and loaded by RCRL
    add_library(repl_plugin SHARED EXCLUDE_FROM_ALL ${plugin_file})
    # link the RCRL plugin with the host app so we can call stuff from it
    target_link_libraries(repl_plugin game)
    # exclude it even for Visual Studio when building the whole solution (EXCLUDE_FROM_ALL is not enough)
    set_target_properties(repl_plugin PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD 1)
    # no 'lib' prefix for some compilers/platforms - simplifies my code
    set_target_properties(repl_plugin PROPERTIES PREFIX "")
    # we don't want .pdb files for the plugin because when we are within Visual Studio and debugging the application it locks
    # the .pdb for the original .dll and subsequent compiles fail even though we have loaded copies of the original .dll
    # can also use /PDBALTPATH - https://blog.molecular-matters.com/2014/05/10/using-runtime-compiled-c-code-as-a-scripting-language-under-the-hood/
    set_target_properties(repl_plugin PROPERTIES LINK_FLAGS /DEBUG:NONE)
    
    # add a precompiled header
    add_precompiled_header(repl_plugin "${CMAKE_CURRENT_SOURCE_DIR}/precompiled.h")
endif()

# ========= REPL END =========

if(TOOLCHAIN STREQUAL "js")
    set_target_properties(game PROPERTIES SUFFIX ".html")
else()
    target_include_directories(game PRIVATE ../third_party/glfw/include)
    target_link_libraries(game glew)
    target_link_libraries(game glfw)
    target_link_libraries(game file_watcher)
endif()
