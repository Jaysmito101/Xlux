include_directories(
    ./Include
    ../KluxEngine/Include
)

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_executable(sandbox
    ./Source/Main.cpp
)

target_link_libraries(sandbox
    PRIVATE kluxengine
)

target_compile_definitions(sandbox
    PUBLIC KLUX_DLL_IMPORT
)

if (WIN32)
    target_compile_definitions(sandbox
        PUBLIC _CRT_SECURE_NO_WARNINGS
    )
endif()

if( MSVC )
    if(${CMAKE_VERSION} VERSION_LESS "3.6.0") 
        message( "\n\t[ WARNING ]\n\n\tCMake version lower than 3.6.\n\n\t - Please update CMake and rerun; OR\n\t - Manually set 'GLFW-CMake-starter' as StartUp Project in Visual Studio.\n" )
    else()
        set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT sandbox )
    endif()
endif()