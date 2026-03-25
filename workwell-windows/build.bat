@echo off
echo ========================================
echo   WorkWell - Build Script
echo ========================================
echo.

where cmake >/dev/null 2>/dev/null
if errorlevel 1 (
    echo Error: CMake not found
    echo Download: https://cmake.org/download/
    pause
    exit /b 1
)

if not exist build mkdir build
cd build

echo [1/2] Configuring...
cmake .. -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto error

echo.
echo [2/2] Building...
cmake --build . --config Release
if errorlevel 1 goto error

cd ..
echo.
echo ========================================
echo   Build Complete!
echo ========================================
echo.
echo Output: build\bin\Release\WorkWell.exe
echo.
goto end

:error
cd ..
echo.
echo Build failed!
echo.

:end
pause
