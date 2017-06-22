# cache this for use inside of the function
set(CURRENT_LIST_DIR_CACHED ${CMAKE_CURRENT_LIST_DIR})

# implementation of add_mix_dirs
macro(add_mix_dir_impl result rec trim dirs_in additional_ext)
    set(dirs "${dirs_in}")
    
    # handle the "" and "." cases
    if("${dirs}" STREQUAL "" OR "${dirs}" STREQUAL ".")
        set(dirs "./")
    endif()
    
    foreach(cur_dir ${dirs})
        # to circumvent some linux/cmake/path issues - barely made it work...
        if(cur_dir STREQUAL "./")
            set(cur_dir "")
        else()
            set(cur_dir "${cur_dir}/")
        endif()
        
        set(additional_file_extensions "${cur_dir}*.mix")
        foreach(ext ${additional_ext})
            list(APPEND additional_file_extensions "${cur_dir}*.${ext}")
        endforeach()
        
        # find all sources and add them to result
        FILE(GLOB found_sources RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
        # sources
            "${cur_dir}*.cpp"
            "${cur_dir}*.c"
        # headers
            "${cur_dir}*.h"
            "${cur_dir}*.hpp"
            "${cur_dir}*.inl"
            ${additional_file_extensions})
        list(APPEND ${result} ${found_sources})
        
        # set the proper filters
        ucm_trim_front_words("${cur_dir}" cur_dir "${trim}")
        # replacing forward slashes with back slashes so filters can be generated (back slash used in parsing...)
        STRING(REPLACE "/" "\\" FILTERS "${cur_dir}")
        SOURCE_GROUP("${FILTERS}" FILES ${found_sources})
        
        # set the filters as a property so it might be queried later
        set_source_files_properties(${found_sources} PROPERTIES FILTER_POP "${FILTERS}")
    endforeach()
    
    if(${rec})
        foreach(cur_dir ${dirs})
            ucm_dir_list("${cur_dir}" subdirs)
            foreach(subdir ${subdirs})
                add_mix_dir_impl(${result} ${rec} ${trim} "${cur_dir}/${subdir}" "${additional_ext}")
            endforeach()
        endforeach()
    endif()
endmacro()

# like ucm_add_dirs but optimized for this codebase - also adds the filters to .mix files as source properties
macro(add_mix_dirs)
    cmake_parse_arguments(ARG "RECURSIVE" "TO;FILTER_POP" "ADDITIONAL_EXT" ${ARGN})
    
    if(${ARG_TO} STREQUAL "")
        message(FATAL_ERROR "Need to pass TO and a variable name to add_mix_dirs()")
    endif()
    
    if("${ARG_FILTER_POP}" STREQUAL "")
        set(ARG_FILTER_POP 0)
    endif()
    
    add_mix_dir_impl(${ARG_TO} ${ARG_RECURSIVE} ${ARG_FILTER_POP} "${ARG_UNPARSED_ARGUMENTS}" "${ARG_ADDITIONAL_EXT}")
endmacro()

# for each .mix file in the target sources a header file will be generated and added to the target sources
function(mixify_target target)
    get_target_property(sources ${target} SOURCES)
	foreach(src ${sources})
        if("${src}" MATCHES \\.\(mix\)$)
            set(mix_file ${CMAKE_CURRENT_SOURCE_DIR}/${src})
            
            get_filename_component(mix_name_only ${mix_file} NAME_WE)
            string(REGEX REPLACE "\\.[^.]*$" "" mix_name_no_ext ${mix_file})
            
            set(gen_header ${mix_name_no_ext}_gen.h)
            
            add_custom_command(
                OUTPUT ${gen_header}
                MAIN_DEPENDENCY ${mix_file}
                COMMAND python ${CURRENT_LIST_DIR_CACHED}/../python/mix_file.py ${mix_file} ${gen_header}
                COMMENT "mixing ${mix_file}")
            
            target_sources(${target} PUBLIC ${gen_header})
            
            get_source_file_property(filters ${mix_file} FILTER_POP)
            SOURCE_GROUP("${filters}" FILES ${gen_header})
        endif()
    endforeach()
endfunction()

# adds a plugin with supposedly .mix files in it - so it also mixifies it
function(add_mixin)
    cmake_parse_arguments(ARG "" "NAME" "LINK_TO" ${ARGN})
    
    if(NOT ${WITH_PLUGINS})
        list(APPEND PLUGIN_SOURCES ${ARG_UNPARSED_ARGUMENTS})
        set(PLUGIN_SOURCES ${PLUGIN_SOURCES} PARENT_SCOPE) # because we are a function
        # TODO: figure out what to do with the ARG_LINK_TO list - should somehow put stuff in a set of unique link libs...
        return()
    endif()
    
    add_library(${ARG_NAME} SHARED ${ARG_UNPARSED_ARGUMENTS})
    set_target_properties(${ARG_NAME} PROPERTIES OUTPUT_NAME ${ARG_NAME}_plugin)
    set_target_properties(${ARG_NAME} PROPERTIES FOLDER "plugins")
    target_link_libraries(${ARG_NAME} dynamix)
    target_link_libraries(${ARG_NAME} utils)
    target_link_libraries(${ARG_NAME} game)
    target_link_libraries(${ARG_NAME} ppk_assert)
    target_link_libraries(${ARG_NAME} registry)
    target_link_libraries(${ARG_NAME} serialization)
    target_link_libraries(${ARG_NAME} doctest_runner)
    
    foreach(lib "${ARG_LINK_TO}")
        target_link_libraries(${ARG_NAME} ${lib})
    endforeach()
    
    # for MSVC make the output name different, and copy to the expected name with a post-build step - this is needed
    # because the MSVC linker modifies a few times the .dll while finalizing it and file watchers go crazy!
    # but with a copy on post-build file watchers see just 1 notification for modification
    if(MSVC)
        set_target_properties(${ARG_NAME} PROPERTIES OUTPUT_NAME ${ARG_NAME}_plugin_msvc_orig)
        add_custom_command(TARGET ${ARG_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${ARG_NAME}>
            $<TARGET_PROPERTY:${ARG_NAME},LIBRARY_OUTPUT_DIRECTORY>/$<CONFIG>/${ARG_NAME}_plugin.dll)
    endif()
    
    target_compile_definitions(${ARG_NAME} PRIVATE
        HARDLY_PLUGIN # this will affect the macros used for registering mixins - for supporting plugin reloading
    )
    
    mixify_target(${ARG_NAME})
    
    # add the plugin to the list of plugins - so the main executable can depend on them
    list(APPEND PLUGINS ${ARG_NAME})
    set(PLUGINS ${PLUGINS} PARENT_SCOPE) # because we are a function
endfunction()

# should be used on libraries (static or dynamic) which will be linked to other dynamic libraries
function(add_fPIC_to_target lib)
    if(NOT WIN32)
        target_compile_options(${lib} PRIVATE -fPIC)
    endif()
endfunction()







# TODO: TEST THIS! got it from here: https://gist.github.com/Zeex/5291602
# group_target_sources(target) - group target's source files on based on their directory.
function(group_target_sources target)
	get_target_property(sources ${target} SOURCES)
	foreach(file ${sources})
		get_filename_component(path "${file}" ABSOLUTE)
		get_filename_component(path "${path}" PATH)
		if(file MATCHES "${PROJECT_BINARY_DIR}")
			file(RELATIVE_PATH path ${PROJECT_BINARY_DIR} "${path}")
		else()
			file(RELATIVE_PATH path ${PROJECT_SOURCE_DIR} "${path}")
		endif()
		string(REGEX REPLACE "/" "\\\\" win_path "${path}")
		source_group("${win_path}" REGULAR_EXPRESSION "${path}/[^/\\]+\\..*")
	endforeach()
endfunction()

































