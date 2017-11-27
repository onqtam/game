#===================================================================================================
#== FLAGS AND DEFINES FOR ALL PROJECTS =============================================================
#===================================================================================================

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    ucm_add_flags(CXX -std=c++1z -m64)
    
    #ucm_add_flags(CONFIG Release -ffast-math -flto)
    #ucm_add_linker_flags(-Wl,--no-undefined) # problematic for glfw?!
endif()

if(MSVC)
    add_definitions(-DWIN32_MEAN_AND_LEAN)
    add_definitions(-DVC_EXTRA_LEAN)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-DNOMINMAX)
    
    ucm_add_flags(/MP)                  # for parallel source file compilation
    ucm_add_flags(/Gm-)                 # disable minimal rebuild (which is incompatible with /MP)
    ucm_add_flags(/GF)                  # Eliminate Duplicate Strings
    ucm_add_flags(/Zc:throwingNew)      # https://blogs.msdn.microsoft.com/vcblog/2015/08/06/new-in-vs-2015-zcthrowingnew/
    
    ucm_add_flags(CONFIG Debug      /Ob1              /D_ITERATOR_DEBUG_LEVEL=2)
    ucm_add_flags(CONFIG Release    /Gw /GL /fp:fast  /D_ITERATOR_DEBUG_LEVEL=0)
    
    ucm_add_flags(/wd4251) # identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
    
    # https://blogs.msdn.microsoft.com/vcblog/2016/10/05/faster-c-build-cycle-in-vs-15-with-debugfastlink/
    
    ucm_add_flags(/std:c++latest)
    
    # TODO: not sure if this should be set globally
    #ucm_add_linker_flags(/ignore:4217) # locally defined symbol 'symbol' imported in function 'function'
    #ucm_add_linker_flags(/ignore:4049) # locally defined symbol 'symbol' imported
    ucm_add_linker_flags(/ignore:4221) # This object file does not define any previously undefined public symbols,
                                       # so it will not be used by any link operation that consumes this library
    
    #ucm_add_linker_flags(/DYNAMICBASE)  #ASLR
    #ucm_add_linker_flags(/NXCOMPAT)     #DEP
    
    # /ltcg:incremental for release
    # https://blogs.msdn.microsoft.com/vcblog/2016/10/26/recommendations-to-speed-c-builds-in-visual-studio/
    # /incremental for debug
    # /debug:fastlink ... for debug? :D
    
    ucm_add_linker_flags(CONFIG Release /LTCG)
endif()

add_definitions(-DDYNAMIX_DYNLIB)
ucm_add_flags(-DDYNAMIX_NO_MSG_THROW CONFIG Release)

# removes the ability to ignore certain asserts after they have fired - requires the use of local static bools...
add_definitions(-DPPK_ASSERT_DISABLE_IGNORE_LINE)
# enable asserts for all builds (for now - todo: real release build)
add_definitions(-DPPK_ASSERT_ENABLED=1)

# this disables the auto-linking of boost libraries (embedded #pragma comment(lib, ...) of some sort in header files... yuck)
add_definitions(-DBOOST_ALL_NO_LIB)
# for building these libraries as shared (BOOST_ALL_DYN_LINK works for all)
add_definitions(-DBOOST_PROGRAM_OPTIONS_DYN_LINK)
add_definitions(-DBOOST_FILESYSTEM_DYN_LINK)

if(WIN32)
    set(ha_symbol_export "__declspec(dllexport)")
    set(ha_symbol_import "__declspec(dllimport)")
else()
    set(ha_symbol_export "__attribute__((visibility(\"default\")))")
    set(ha_symbol_import "")
endif()

#===================================================================================================
#== HARDLY FLAGS AND DEFINES =======================================================================
#===================================================================================================

if(MSVC)
    list(APPEND ha_compiler_flags
        #/WX
        /W4
        # some warnings are disabled by default even if on levels 1/2/3 (also 4) - so I set them explicitly as level 3
        # https://msdn.microsoft.com/en-us/library/23k5d385.aspx
        # https://www.reddit.com/r/cpp/comments/70hf7f/useful_gcc_warning_options_not_enabled_by_wall/dn37eh7/
        
        # "expression without effect" related
        /w34296
        /w34545
        /w34546
        /w34547
        /w34548
        /w34549
        
        # override related
        /w34263
        /w34264
        /w34266
        
        #/wd4661 # identifier' : no suitable definition provided for explicit template instantiation request
    )
    list(APPEND ha_third_party_compiler_flags /W0)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    list(APPEND ha_compiler_flags
        #-Werror
        -pedantic
        #-pedantic-errors
        -fvisibility=hidden # TODO: add this globally - not just to ha_compiler_flags
        -fstrict-aliasing
    )
    list(APPEND ha_third_party_compiler_flags -w)
endif()    

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    
    ucm_add_flags(-stdlib=libc++)
    
    list(APPEND ha_compiler_flags
        -Weverything
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Qunused-arguments
        -fcolor-diagnostics # needed for ccache integration
        
        -Wno-padded
        -Wno-exit-time-destructors
        -Wno-global-constructors
        -Wno-weak-vtables
        
        -Wno-disabled-macro-expansion # TODO: wtf why can't I use this warning?! stderr doesn't expand properly in emscripten...
        
        # disable these for now...
        -Wno-sign-conversion
        -Wno-old-style-cast
        -Wno-missing-prototypes
        -Wno-unknown-pragmas
    )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    list(APPEND ha_compiler_flags
        -Wall
        -Wextra
        -fdiagnostics-show-option
        -Wfloat-equal
        -Wlogical-op
        #-Wundef            # dont know why this cant be silenced for third party headers with a pragma...
        #-Wredundant-decls  # is this needed? problematic when using DYNAMIX_DECLARE_MIXIN twice...
        -Wshadow
        -Wwrite-strings
        -Wpointer-arith
        -Wformat=2
        -Wmissing-include-dirs
        -Wcast-align
        -Wnon-virtual-dtor
        -Wctor-dtor-privacy
        -Winvalid-pch
        -Wmissing-declarations
        -Woverloaded-virtual
        -Wnoexcept
        -Wtrampolines
        -Wzero-as-null-pointer-constant
        -Wuseless-cast
        -Wsized-deallocation
        -Wshift-overflow=2
        -Wnull-dereference # requires -fdelete-null-pointer-checks which is enabled by optimizations in most targets
        -Wduplicated-cond
        -Wold-style-cast
        -Wswitch-default
        -Wswitch-enum
        -Wcast-qual
        -Wdouble-promotion
        -Wconversion
        -Wsign-conversion
        -Wstrict-overflow=4 # read about it here: https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
        
        # disable these for now...
        -Wno-sign-conversion
        -Wno-conversion
        -Wno-old-style-cast
        -Wno-missing-declarations
        -Wno-strict-aliasing
        -Wno-double-promotion
        
        # optimizations related
        #-Wstack-protector # requires -fstack-protector-all
        #-Wunsafe-loop-optimizations # requires -funsafe-loop-optimizations
        #-Wvector-operation-performance
        #-Wdisabled-optimization
    )
    
    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
        list(APPEND ha_compiler_flags
            -Wduplicated-branches
            -Wrestrict
            -Walloc-zero
            -Walloca
        )
    endif()
        
endif()

















