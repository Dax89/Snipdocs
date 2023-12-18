if(NOT mymodule_FOUND)
    find_path(mymodule_INCLUDE_DIR mymodule.h)
    find_library(mymodule_LIBRARIES mymodulelib)

    if(mymodule_INCLUDE_DIR AND mymodule_LIBRARIES)
        set(mymodule_FOUND TRUE)
    endif()

    if(mymodule_FOUND)
        if (NOT mymodule_FIND_QUIETLY)
            message(STATUS "Found mymodule\n\tmymodule_INCLUDE_DIR: ${mymodule_INCLUDE_DIR}\n\tmymodule_LIBRARIES: ${mymodule_LIBRARIES}")
        endif()

        if(NOT TARGET mymodule)
            add_library(mymodule INTERFACE IMPORTED GLOBAL)
            target_include_directories(mymodule INTERFACE ${mymodule_INCLUDE_DIR})
            target_link_libraries(mymodule INTERFACE ${mymodule_LIBRARIES})
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
