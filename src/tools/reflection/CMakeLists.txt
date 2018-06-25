
# YARS - yet another reflection solution
# = reflection repos:
# https://github.com/foonathan/cppast
# https://github.com/Manu343726/tinyrefl
# https://github.com/Manu343726/siplasplas
# https://github.com/AustinBrunkhorst/CPP-Reflection/
#     https://www.reddit.com/r/gamedev/comments/3lh0ba/using_clang_to_generate_c_reflection_data/
#     https://blog.austinbrunkhorst.com/cpp-reflection-part-1/
# https://github.com/chakaz/reflang
#     https://shaharmike.com/cpp/libclang/
# = and some wtf ones:
# https://github.com/TReed0803/RefN/
#     http://www.trentreed.net/blog/refn-crafting-a-scalable-reflection-library-part-1/
# https://github.com/eliben/llvm-clang-samples
# = others
# https://www.youtube.com/watch?v=E6i8jmiy8MY
# P. Goldsborough “clang-useful: Building useful tools with LLVM and clang for fun and profit"

if(WIN32 OR TOOLCHAIN STREQUAL "js")
    set(LLVM_CONFIG true)
    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        set(CLANG_INCLUDEDIR "C:/Program Files (x86)/LLVM/include")
        set(CLANG_LIBDIR "C:/Program Files (x86)/LLVM/lib")
        set(CLANG_ROOT "C:/Program Files (x86)/LLVM")
    else()
        set(CLANG_INCLUDEDIR "C:/Program Files/LLVM/include")
        set(CLANG_LIBDIR "C:/Program Files/LLVM/lib")
        set(CLANG_ROOT "C:/Program Files/LLVM")
    endif()
    set(CLANG_LIBS ${CLANG_LIBDIR}/libclang.lib)
endif()

find_package(LibClang REQUIRED)

include_directories(${CLANG_INCLUDEDIR})

add_executable(reflection
    main.cpp
    parser.class.cpp
    parser.enum.cpp
    parser.function.cpp
    parser.hpp
    parser.cpp
    parser.util.hpp
    parser.util.cpp
    types.hpp
    types.cpp
)

target_link_libraries(reflection ${CLANG_LIBS} mustache)
set_target_properties(reflection PROPERTIES FOLDER "tools")
