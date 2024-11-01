include_directories(
    ../imgui
)

add_library(imgui
    ../imgui/imgui.cpp
    ../imgui/imgui_demo.cpp
    ../imgui/imgui_draw.cpp
    ../imgui/imgui_widgets.cpp
    ../imgui/imgui_tables.cpp
)

if (WIN32)
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
    )
endif()

if( MSVC )
    if(${CMAKE_VERSION} VERSION_LESS "3.6.0") 
        message( "\n\t[ WARNING ]\n\n\tCMake version lower than 3.6.\n\n\t - Please update CMake and rerun; OR\n\t - Manually set 'GLFW-CMake-starter' as StartUp Project in Visual Studio.\n" )
    else()
        set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT sandbox )
    endif()
endif()