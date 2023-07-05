find_package(Qt6 6.2 
    COMPONENTS 
        Quick
        Gui 
        QuickControls2
        
    REQUIRED)

qt_standard_project_setup(REQUIRES 6.5)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

macro(configure_qtquick_libraries)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            Qt6::Gui
            Qt6::Quick
            Qt6::QuickControls2
    )
endmacro()
