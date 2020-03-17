@echo off
echo Start build BEngine project
SET PLATFORM="windows"

echo Make %PLATFORM% proejct

if exist %pwd%/%PLATFORM%/ (
  echo clean %PLATFORM% dir
  rmdir %PLATFORM%
)

mkdir %PLATFORM%
cd %PLATFORM%
cmake -A Win64 .. 
cd ..
