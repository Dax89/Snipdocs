find_package(FLTK 1.3)

if(FLTK_FOUND)
    target_include_directories(${PROJECT_NAME}
        PRIVATE
            ${FLTK_INCLUDE_DIR}
    )

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            ${FLTK_LIBRARIES}
    )
else()
    set(FLTK_LIBRARIES "") 

    CPMAddPackage(FLTK
        GITHUB_REPOSITORY "fltk/fltk"
        # GIT_TAG "master"
        GIT_TAG "release-1.3.8"
        OPTIONS
            "FLTK_BUILD_TEST OFF"
            "FLTK_BUILD_EXAMPLES OFF"
            "FLTK_BUILD_FLUID OFF"
            "OPTION_USE_SYSTEM_ZLIB OFF"
            "OPTION_USE_SYSTEM_LIBJPEG OFF"
            "OPTION_USE_SYSTEM_LIBPNG OFF"
            "OPTION_USE_GDIPLUS OFF"
            "OPTION_USE_GL OFF" # Depends on "GL/glu.h"
            "OPTION_BUILD_HTML_DOCUMENTATION OFF"
            "OPTION_INSTALL_HTML_DOCUMENTATION OFF"
    )

    target_include_directories(${PROJECT_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${FLTK_BINARY_DIR}>
            $<BUILD_INTERFACE:${FLTK_SOURCE_DIR}>
    )

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            fltk
    )
endif()

if(WIN32)
    set(GUI_TYPE WIN32)
elseif(APPLE)
    set(GUI_TYPE MACOSX_BUNDLE)

    target_link_libraries(${PROJECT_NAME}  # needed for Darwin
        PRIVATE
            "-framework Cocoa"
    )
else()
    set(GUI_TYPE)
endif()
