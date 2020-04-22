@echo off
echo Start build BEngine project
set PLATFORM="windows"
echo Make %PLATFORM% proejct

:: check vcpkg
if not exist %cd%\vcpkg\ (
  echo Download vcpkg project ...
  git clone "https://github.com/Microsoft/vcpkg.git"
) else (
  echo Already vcpkg project donwload ...
)

cd vcpkg
if not exist %cd%/vcpkg.exe (
  echo Build vcpkg ...
  call bootstrap-vcpkg.bat
) else (
  echo Already build vcpkg porject ...
)

for /f "delims=" %%A in ('vcpkg list ^| findstr "glfw3:x64-windows"') do (set exist_glfw3=%%A)
if "%exist_glfw3%" EQU "" (
  echo Install glfw3 package ...
  call vcpkg install glfw3:x64-windows
) else (
  echo Already installed glfw3 package ...
)

for /f "delims=" %%A in ('vcpkg list ^| findstr "vulkan:x64-windows"') do (set exist_vulkan=%%A)
if "%exist_vulkan%" EQU "" (
  echo Install vulkan package ...
  call vcpkg install vulkan:x64-windows
) else (
  echo Already installed vulkan package ...
)

:: double check vulkan package
:: vcpkg 에서 vulkan 패키지는 미리 설치된 패키지를 참조하는 식으로 동작한다.
for /f "delims=" %%A in ('vcpkg list ^| findstr "vulkan:x64-windows"') do (set exist_vulkan=%%A)
if "%exist_vulkan%" EQU "" (
  echo Need to Install vulkan sdk ...
  echo Please download vulkan sdk : https://vulkan.lunarg.com/sdk/home
  echo And Install, new cmd open, restart build.bat
  cd ..
  goto QUIT
)
cd ..

if not exist %pwd%/%PLATFORM%/ (
  mkdir %PLATFORM%
)


set TOOLCHAIN_PATH="%cd%\vcpkg\scripts\buildsystems\vcpkg.cmake"
cd %PLATFORM%
cmake -G "Visual Studio 15 2017 Win64" ^
    -DCMAKE_TOOLCHAIN_FILE=%TOOLCHAIN_PATH% ^
    ..
cd ..


:QUIT
echo Finish build.bat