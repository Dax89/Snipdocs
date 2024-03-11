find_package(PkgConfig)

if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_mymodule QUIET mymodule)
endif()

if(PC_mymodule_FOUND)
    message(STATUS "Found mymodule pkg-config\n\tPC_mymodule_INCLUDE_DIR: ${PC_mymodule_INCLUDE_DIRS}\n\tPC_mymodule_LIBRARIES: ${PC_mymodule_LIBRARIES}")
    set(mymodule_FOUND "${PC_mymodule_FOUND}")
    set(mymodule_INCLUDE_DIR "${PC_mymodule_INCLUDE_DIRS}")
    set(mymodule_LIBRARIES "${PC_mymodule_LIBRARIES}")
else()
    find_path(mymodule_INCLUDE_DIR NAMES mymodule.h)
    find_library(mymodule_LIBRARIES NAMES mymodule)

    if(mymodule_INCLUDE_DIR AND mymodule_LIBRARIES)
        set(mymodule_FOUND TRUE)
    endif()

    if(mymodule_FOUND)
        if(NOT mymodule_FIND_QUIETLY)
            message(STATUS "Found mymodule\n\tmymodule_INCLUDE_DIR: ${mymodule_INCLUDE_DIR}\n\tmymodule_LIBRARIES: ${mymodule_LIBRARIES}")
        endif()
    elseif(mymodule_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find mymodule")
    endif()

    mark_as_advanced(
        mymodule_FOUND
        mymodule_INCLUDE_DIR
        mymodule_LIBRARIES
    )
endif()

if(NOT TARGET mymodule)
    add_library(mymodule INTERFACE IMPORTED GLOBAL)
    target_include_directories(mymodule INTERFACE ${mymodule_INCLUDE_DIR})
    target_link_libraries(mymodule INTERFACE ${mymodule_LIBRARIES})
    # add_library(mymodule::mymodule ALIAS mymodule)
endif()
