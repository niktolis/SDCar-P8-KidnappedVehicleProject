@echo off

setlocal

set "CarNDPFProjectPlatform=x64"
set "CarNDPFProjectToolset=v141"
set "CarNDPFProjectBuildType=Debug"

if NOT "%~1"=="" set "CarNDPFProjectPlatform=%~1"
if NOT "%~2"=="" set "CarNDPFProjectToolset=%~2"
if NOT "%~3"=="" set "CarNDPFProjectBuildType=%~3" 

set "VcPkgDir=c:\vcpkg"
set "VcPkgTriplet=%CarNDPFProjectPlatform%-windows"
rem set "VcPkgTriplet=%CarNDPFProjectPlatform%-windows-%CarNDPFProjectToolset%"

if defined VCPKG_ROOT_DIR if /i not "%VCPKG_ROOT_DIR%"=="" (
    set "VcPkgDir=%VCPKG_ROOT_DIR%"
)
if defined VCPKG_DEFAULT_TRIPLET if /i not "%VCPKG_DEFAULT_TRIPLET%"=="" (
    set "VcpkgTriplet=%VCPKG_DEFAULT_TRIPLET%"
)
set "VcPkgPath=%VcPkgDir%\vcpkg.exe"

echo. & echo Bootstrapping dependencies for triplet: %VcPkgTriplet% & echo.

rem ==============================
rem Update and Install packages
rem ==============================
call "%VcPkgPath%" update

rem Install latest uwebsockets
call "%VcPkgPath%" install uwebsockets --triplet %VcPkgTriplet%
rem Use adapted main.cpp for latest uwebsockets
rem copy main.cpp ..\..\src

rem ==============================
rem Configure CMake
rem ==============================

set "VcPkgTripletDir=%VcPkgDir%\installed\%VcPkgTriplet%"

set "CMAKE_PREFIX_PATH=%VcPkgTripletDir%;%CMAKE_PREFIX_PATH%"

echo. & echo Bootstrapping successful for triplet: %VcPkgTriplet% & echo CMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH% & echo.

set "CarNDPFProjectCMakeGeneratorName=Visual Studio 15 2017"

if "%CarNDPFProjectPlatform%"=="x86" (
    if "%CarNDPFProjectToolset%"=="v140" set "CarNDPFProjectCMakeGeneratorName=Visual Studio 14 2015"
    if "%CarNDPFProjectToolset%"=="v141" set "CarNDPFProjectCMakeGeneratorName=Visual Studio 15 2017"
)

if "%CarNDPFProjectPlatform%"=="x64" (
    if "%CarNDPFProjectToolset%"=="v140" set "CarNDPFProjectCMakeGeneratorName=Visual Studio 14 2015 Win64"
    if "%CarNDPFProjectToolset%"=="v141" set "CarNDPFProjectCMakeGeneratorName=Visual Studio 15 2017 Win64"
)

set "CarNDPFProjectBuildDir=%~dp0\..\..\products\cmake.msbuild.windows.%CarNDPFProjectPlatform%.%CarNDPFProjectToolset%"
if not exist "%CarNDPFProjectBuildDir%" mkdir "%CarNDPFProjectBuildDir%"
cd "%CarNDPFProjectBuildDir%"

echo: & echo CarNDPFProjectBuildDir=%CD% & echo cmake.exe -G "%CarNDPFProjectCMakeGeneratorName%" "%~dp0\..\.." & echo:

call cmake.exe -G "%CarNDPFProjectCMakeGeneratorName%" "%~dp0\..\.."

call "%VcPkgPath%" integrate install

endlocal