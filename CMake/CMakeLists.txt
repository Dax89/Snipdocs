cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(my_project)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules")

include(cmake/CPM.cmake) # https://github.com/TheLartians/CPM.cmake

#CPMAddPackage("gh:TartanLlama/optional@1.1.0")
#CPMAddPackage("gh:TartanLlama/expected@1.1.0")
#CPMAddPackage("gh:google/re2#2023-02-01")
#CPMAddPackage("gh:eliaskosunen/scnlib@1.1.2")
CPMAddPackage("gh:fmtlib/fmt#9.1.0")
CPMAddPackage("gh:gabime/spdlog@1.11.0")

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        fmt
        spdlog
        #scn::scn
        #optional
        #expected
        #re2
)

target_sources(${PROJECT_NAME}
    PUBLIC
        # Add files here...

    PRIVATE
        # Add files here...
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED TRUE
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_definitions(${PROJECT_NAME} PUBLIC "_GLIBCXX_DEBUG")
    endif()

    target_compile_options(${PROJECT_NAME}
        PUBLIC
            "-Wall"                     # essential
            "-Wextra"                   # essential
            "-Werror"                   # essential
            "-Wpedantic"                # essential
            "-Wfloat-equal"             # because usually testing floating-point numbers for equality is bad
            "-Wwrite-strings"           # give string constants the type const char[length] so that copying the address of one into a non-const char * pointer will get a warning
            "-Wsign-compare"            # determines whether warnings are issued when a comparison between signed and unsigned values could produce an incorrect result
            "-Wpointer-arith"           # warn if anything depends upon the size of a function or of void.
            "-Wshadow"                  # warn whenever a local variable shadows another local variable, parameter or global variable 
            "-Wunreachable-code"        # warn if the compiler detects that code will never be executed.
            "-Wduplicated-cond"         # warn about duplicated condition in if-else-if chains
            "-Wnull-dereference"        # warn when the compiler detects paths that dereferences a null pointer.
            "-Wdouble-promotion"        # warn when a value of type float is implicitly promoted to double
            # "-O2"                       # enable standard optimizations
            "-Wmaybe-uninitialized"     
            "-Wuninitialized"           
            "-Wunused"                  
            "-Wunused-parameter"        
            "-Wunused-value"            
            "-Wunused-variable"
            "-Wunused-local-typedefs"   
            "-Wunused-but-set-parameter"
            "-Wunused-but-set-variable"
            "-g"
            "-ggdb"
            # "-fsanitize=undefined"      # undefined behavior flags
            "-fsanitize=address"        # address sanitizer flags
            "-fno-omit-frame-pointer"   # address sanitizer flags

            # Report only
            "-Wno-error=unused"                  
            "-Wno-error=unused-function"        
            "-Wno-error=unused-parameter"        
            "-Wno-error=unused-value"            
            "-Wno-error=unused-variable"
            "-Wno-error=unused-local-typedefs"   
            "-Wno-error=unused-but-set-parameter"
            "-Wno-error=unused-but-set-variable"
    )

    target_link_options(${PROJECT_NAME}
        PUBLIC
            # "-fno-exceptions"
            "-fno-rtti"

        PUBLIC
            # "-fsanitize=undefined"
            "-fsanitize=address"
            "-fno-omit-frame-pointer"
    )
endif()
