@echo off
echo Formatting C++ files...
for /f "delims=" %%f in ('dir /s /b *.c *.cpp *.h *.hpp 2^>nul ^| findstr /v /i "\\Vendor\\ stb_image cgltf tiny_obj_loader \\build\\ \\build.ninja\\ "') do (
    echo Formatting: %%f
    clang-format -i "%%f"
)
echo Done!