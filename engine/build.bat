@echo off
echo Start build BEngine project
set PLATFORM="windows"

set CURRENT_DIR=%cd%
set VCPKG_PATH=%VCPKG_PATH%

echo %VCPKG_PATH%
echo Make %PLATFORM% proejct

if "%VCPKG_PATH%" == "" (
  echo "Install vcpkg"
  git clone "https://github.com/Microsoft/vcpkg.git"
  cd vcpkg
  call bootstrap-vcpkg.bat
  
  echo "Install glfw3 package"
  call vcpkg install glfw3:x64-windows

  echo "Install vulkan package"
  call vcpkg install vulkan:x64-windows

  setx VCPKG_PATH %cd%
  echo "Finished install vcpkg. Please new cmd open and restart build.bat file"
)

if not exist %pwd%/%PLATFORM%/ (
  mkdir %PLATFORM%
)

echo %cd%
set TOOLCHAIN_PATH="%VCPKG_PATH%\scripts\buildsystems\vcpkg.cmake"
cd %PLATFORM%
cmake -G "Visual Studio 15 2017 Win64" ^
    -DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_PATH% ^
    ..
cd ..
