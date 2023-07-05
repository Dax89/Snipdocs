find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

CPMAddPackage(
    NAME imgui
    GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
    GIT_TAG "master"
    DOWNLOAD_ONLY TRUE
)

add_library(imgui)

target_sources(imgui
    PRIVATE
        "${imgui_SOURCE_DIR}/imgui.cpp"
        "${imgui_SOURCE_DIR}/imgui.h"
        "${imgui_SOURCE_DIR}/imconfig.h"
        "${imgui_SOURCE_DIR}/imgui_demo.cpp"
        "${imgui_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_SOURCE_DIR}/imgui_internal.h"
        "${imgui_SOURCE_DIR}/imgui_tables.cpp"
        "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
        "${imgui_SOURCE_DIR}/imstb_rectpack.h"
        "${imgui_SOURCE_DIR}/imstb_textedit.h"
        "${imgui_SOURCE_DIR}/imstb_truetype.h"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl2.h"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl2.cpp"
)

target_include_directories(imgui 
    PUBLIC
        "${imgui_SOURCE_DIR}"
)

target_link_libraries(imgui 
    PUBLIC 
        OpenGL::GL
        glfw
)
