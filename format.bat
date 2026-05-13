@echo off
echo Formatting C++ files...
for /f "delims=" %%f in ('dir /s /b *.c *.cpp *.h *.hpp 2^>nul ^| findstr /v /i "\\Vendor\\"') do (
    clang-format -i "%%f"
)
echo Done!