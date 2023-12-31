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
    ./Source/Core/LinearAllocator.cpp
# impl
    ./Source/Impl/XluxDevice.cpp
    ./Source/Impl/XluxDeviceMemory.cpp
    ./Source/Impl/XluxBuffer.cpp
    ./Source/Impl/XluxPipeline.cpp
    ./Source/Impl/XluxVertexShaderWorker.cpp
    ./Source/Impl/XluxFragmentShaderWorker.cpp
    ./Source/Impl/XluxFrameClearWorker.cpp
    ./Source/Impl/XluxTexture.cpp
    ./Source/Impl/XluxRenderer.cpp
)

add_library(xluxengine SHARED ${SOURCES})
add_library(xluxengine-static STATIC ${SOURCES})

target_compile_definitions(xluxengine
    PUBLIC XLUX_DLL_EXPORT
)

target_compile_definitions(xluxengine-static
    PUBLIC XLUX_STATIC
)


if (WIN32)
    target_compile_definitions(xluxengine
        PUBLIC _CRT_SECURE_NO_WARNINGS
    )

    target_compile_definitions(xluxengine-static
        PUBLIC _CRT_SECURE_NO_WARNINGS
    )
endif()
