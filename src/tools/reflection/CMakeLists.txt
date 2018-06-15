
if(WIN32)
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
