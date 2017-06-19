#ucm_add_flags(${hardly_compiler_flags})

include_directories("./")

# == SHADERS =======================================================================================

if(NOT TOOLCHAIN STREQUAL "js")
    FILE(GLOB_RECURSE shaders_vs ${CMAKE_SOURCE_DIR}/data/shaders/raw/*_vs.sc)
    foreach(vs ${shaders_vs})
        add_shader(${vs} VERTEX   OUTPUT ${CMAKE_SOURCE_DIR}/data/shaders GLSL 130 PLATFORMS glsl gles)
    endforeach()
    
    FILE(GLOB_RECURSE shaders_fs ${CMAKE_SOURCE_DIR}/data/shaders/raw/*_fs.sc)
    foreach(fs ${shaders_fs})
        add_shader(${fs} FRAGMENT OUTPUT ${CMAKE_SOURCE_DIR}/data/shaders GLSL 130 PLATFORMS glsl gles)
    endforeach()
    
    # add the shaders to the project so they are actually included in the build...
    set(SOURCES ${shaders_vs} ${shaders_fs})
endif()

# == GAME ==========================================================================================

if(TOOLCHAIN STREQUAL "js")
    # this is needed because the creation of the pch cant find the include paths even when
    # target_include_directories is used (or interface stuff from target_link_libraries)
    include_directories("../third_party/dynamix/include")
    include_directories("../third_party/ppk_assert")
    include_directories("../third_party/doctest/doctest")
    include_directories("../third_party/glm")
endif()

add_definitions(-DDOCTEST_CONFIG_IMPLEMENTATION_IN_DLL) # imported doctest symbols - test runner in a separate dll

add_mix_dirs(TO utils_src "utils" FILTER_POP 2)
add_library(utils STATIC ${utils_src})
target_link_libraries(utils PUBLIC doctest glm ppk_assert)
add_fPIC_to_target(utils)

add_mix_dirs(TO serialization_src "core/serialization" FILTER_POP 2)
ucm_add_target(NAME serialization TYPE SHARED SOURCES "${serialization_src}")
add_fPIC_to_target(serialization)
target_link_libraries(serialization sajson)

add_mix_dirs(TO registry_src "core/registry" FILTER_POP 2)
add_library(registry STATIC ${registry_src})
target_link_libraries(registry PUBLIC utils sajson dynamix)
add_fPIC_to_target(registry)

# cotire requires the appropriate cpp file for the precompiled header to be the first file in the list of sources
ucm_add_files(TO SOURCES "precompiled.cpp")
ucm_add_files(TO SOURCES "precompiled.h")
ucm_add_files(TO SOURCES "main.cpp")

add_mix_dirs(TO SOURCES "core")
add_mix_dirs(TO SOURCES "mixins/mixins")

add_mix_dirs(TO messages_src "mixins/messages")
add_library(messages SHARED ${messages_src})
target_link_libraries(messages dynamix sajson)

add_library(doctest_runner SHARED utils/doctest/doctest_runner.cpp)
target_link_libraries(doctest_runner PUBLIC doctest)

# ========= PLUGINS =============

add_mix_dirs(TO dummy_src "plugins/dummy" FILTER_POP 2)
add_mixin(NAME dummy ${dummy_src} LINK_TO messages)

add_mix_dirs(TO omg_src "plugins/omg" FILTER_POP 2)
add_mixin(NAME omg ${omg_src} LINK_TO messages)

# ========= PLUGINS END =========

if(NOT MINGW)
    ucm_add_target(NAME game TYPE EXECUTABLE CPP_PER_UNITY 10 PCH_FILE "precompiled.h" SOURCES "${SOURCES};${PLUGIN_SOURCES}" UNITY_EXCLUDED "precompiled.cpp")
else()
    ucm_add_target(NAME game TYPE EXECUTABLE UNITY SOURCES "${SOURCES};${PLUGIN_SOURCES}")
    
    # We get the sources from the target instead of using ${SOURCES} because the target may have been made into a unity
    get_target_property(sources_for_game_target game SOURCES)
    # For MinGW we just insert the "precompiled.h" header as a prefix header just like cotire does. We cannot use a real
    # pch because there is a bug in g++(4.8.2)/mingw toolchain (cc1plus.exe crashes with big pch (around 150mb and above))
    # http://sourceforge.net/mailarchive/message.php?msg_id=30821409
    foreach(curr_source ${sources_for_game_target})
        ucm_include_file_in_sources(${curr_source} HEADER precompiled.h)
    endforeach()
endif()

mixify_target(game)

foreach(plugin ${PLUGINS})
    add_dependencies(game ${plugin})
endforeach()

target_link_libraries(game bgfx ib-compress)

target_link_libraries(game messages)
target_link_libraries(game dynamix)
target_link_libraries(game glm)
target_link_libraries(game ppk_assert)
target_link_libraries(game imgui)
target_link_libraries(game registry)
target_link_libraries(game utils)
target_link_libraries(game serialization)
target_link_libraries(game doctest_runner)

if(TOOLCHAIN STREQUAL "js")
    set_target_properties(game PROPERTIES SUFFIX ".html")
else()
    target_include_directories(game PRIVATE ../third_party/glfw/include)
    target_link_libraries(game glfw)
    target_link_libraries(game file_watcher)
endif()
