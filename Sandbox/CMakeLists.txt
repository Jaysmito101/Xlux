include_directories(
    ./Include
    ../XluxEngine/Include
    ./Vendor/imgui
    ./Vendor/imgui/backends
)

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

link_libraries(
    xluxengine-static
)

# add_compile_definitions(
#     XLUX_DLL_IMPORT
# )


add_executable(sandbox
    ./Source/Sandbox.cpp
    ./Source/Win32_Window.cpp
    ./Source/WindowFramebuffer.cpp
)

add_executable(01_hellotriangle
    ./Source/01_HelloTriangle.cpp
    ./Source/Win32_Window.cpp
    ./Source/WindowFramebuffer.cpp
)

add_executable(02_textures
    ./Source/02_Textures.cpp
    ./Source/Win32_Window.cpp
    ./Source/WindowFramebuffer.cpp
)

add_executable(03_transforms
	./Source/03_Transforms.cpp
	./Source/Win32_Window.cpp
	./Source/WindowFramebuffer.cpp
)

add_executable(04_going3d
	./Source/04_Going3D.cpp
	./Source/Win32_Window.cpp
	./Source/WindowFramebuffer.cpp
)

add_executable(05_modelloading
	./Source/05_ModelLoading.cpp
	./Source/Win32_Window.cpp
	./Source/WindowFramebuffer.cpp
)

add_executable(06_pbr
	./Source/06_PBR.cpp
	./Source/Win32_Window.cpp
	./Source/WindowFramebuffer.cpp
)

add_subdirectory(./Vendor/imgui_cmake)
add_executable(07_imgui
	./Source/07_ImGui.cpp
	./Source/Win32_Window.cpp
	./Source/WindowFramebuffer.cpp
    ./Source/imgui_impl_xlux.cpp
    ./Vendor/imgui/backends/imgui_impl_win32.cpp
)
target_link_libraries(07_imgui imgui)

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