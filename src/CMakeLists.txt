#ucm_add_flags(${hardly_compiler_flags})

include_directories("./")
include_directories("./bgfx_common")

add_definitions(-DDOCTEST_CONFIG_IMPLEMENTATION_IN_DLL) # imported doctest symbols - test runner in a separate dll

add_mix_dirs(TO utils_src "utils" FILTER_POP 2)
add_library(utils STATIC ${utils_src})
target_link_libraries(utils PUBLIC doctest)
target_link_libraries(utils PUBLIC ppk_assert)
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

#ucm_add_files(TO SOURCES "../third_party/bgfx/examples/17-drawstress/drawstress.cpp" FILTER_POP 5)
#ucm_add_files(TO SOURCES "../third_party/bgfx/examples/00-helloworld/helloworld.cpp" FILTER_POP 5)

add_mix_dirs(TO SOURCES "bgfx_common" RECURSIVE)

#ucm_add_files(TO SOURCES "../third_party/bgfx/examples/common/imgui/imgui.cpp" FILTER_POP 6)
#ucm_add_files(TO SOURCES "../third_party/bgfx/examples/common/imgui/ocornut_imgui.cpp" FILTER_POP 6)
#ucm_add_files(TO SOURCES "../third_party/bgfx/examples/common/nanovg/nanovg_bgfx.cpp" FILTER_POP 6)
#ucm_add_files(TO SOURCES "../third_party/bgfx/examples/common/nanovg/nanovg.cpp" FILTER_POP 6)

#add_executable(game ${SOURCES})
#target_link_libraries(game example-common bgfx-vertexdecl)
#target_include_directories(game PUBLIC ${BGFX_DIR}/examples/common ${BGFX_DIR}/3rdparty)
#target_include_directories(game PUBLIC ${BGFX_DIR}/include)
#target_include_directories(game PUBLIC ${BIMG_DIR}/include)
#target_include_directories(game PUBLIC ${BX_DIR}/include)

add_mix_dirs(TO SOURCES "core")
ucm_remove_directories(FROM SOURCES "core/serialization")
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
target_include_directories(game PUBLIC
    #${BGFX_DIR}/examples/common
    ${BGFX_DIR}/examples/00-helloworld
    ${BGFX_DIR}/3rdparty
)

target_link_libraries(game messages)
target_link_libraries(game dynamix)
target_link_libraries(game ppk_assert)
target_link_libraries(game imgui)
target_link_libraries(game registry)
target_link_libraries(game utils)
target_link_libraries(game serialization)
target_link_libraries(game doctest_runner)

if(NOT TOOLCHAIN STREQUAL "js")
    target_compile_definitions(game PRIVATE "-DENTRY_CONFIG_USE_GLFW=1")
    target_include_directories(game PRIVATE ../third_party/glfw/include)
    target_link_libraries(game glfw)
    target_link_libraries(game file_watcher)
else()
    set_target_properties(game PROPERTIES SUFFIX ".html")
endif()
