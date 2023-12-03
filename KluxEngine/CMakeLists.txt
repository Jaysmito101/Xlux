include_directories(
    ./Include
)

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

set(SOURCES
# core
    ./Source/Core/Logger.cpp
    ./Source/Core/Utils.cpp
# impl
    ./Source/Impl/KluxDevice.cpp
    ./Source/Impl/KluxDeviceMemory.cpp
)

add_library(kluxengine SHARED ${SOURCES})
add_library(kluxengine-static STATIC ${SOURCES})

target_compile_definitions(kluxengine
    PUBLIC KLUX_DLL_EXPORT
)

target_compile_definitions(kluxengine-static
    PUBLIC KLUX_STATIC
)


if (WIN32)
    target_compile_definitions(kluxengine
        PUBLIC _CRT_SECURE_NO_WARNINGS
    )

    target_compile_definitions(kluxengine-static
        PUBLIC _CRT_SECURE_NO_WARNINGS
    )
endif()
