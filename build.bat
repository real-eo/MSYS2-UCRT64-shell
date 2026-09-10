@echo off
setlocal enabledelayedexpansion

:: ============================================================
:: build.bat - Configure and build the MSYS2 UCRT64 shell DLL
:: ============================================================
::
:: Usage:
::   build.bat                  -> Debug build (default)
::   build.bat release          -> Release build
::   build.bat clean            -> Delete build directory and exit
::   build.bat rebuild          -> Clean, then Debug build
::   build.bat release rebuild  -> Clean, then Release build
::
:: Always targets x64, since Explorer.exe is a 64-bit process
:: on 64-bit Windows and can only load 64-bit COM DLLs.

set "ROOT=%~dp0"
set "BUILD_DIR=%ROOT%build"
set "CONFIG=Debug"
set "DO_CLEAN=0"
set "EXIT_CODE=1"

:: ---- stop file-explorer ----
taskkill /f /im explorer.exe >nul 2>&1

:: ---- parse args ----
for %%A in (%*) do (
    if /I "%%A"=="release" set "CONFIG=Release"
    if /I "%%A"=="debug"   set "CONFIG=Debug"
    if /I "%%A"=="clean"   set "DO_CLEAN=1"
    if /I "%%A"=="rebuild" set "DO_CLEAN=1"
)

:: ---- clean if requested ----
if "%DO_CLEAN%"=="1" (
    if exist "%BUILD_DIR%" (
        echo Removing "%BUILD_DIR%" ...
        rmdir /s /q "%BUILD_DIR%"
    )
    if /I "%~1"=="clean" (
        echo Clean complete.
        set "EXIT_CODE=0"
        goto :exit
    )
)

:: ---- locate cmake ----
:: Prefer whatever's already on PATH (e.g. a standalone CMake install).
set "CMAKE_EXE="
where cmake >nul 2>nul
if not errorlevel 1 (
    set "CMAKE_EXE=cmake"
)

:: Otherwise, fall back to the copy bundled inside Visual Studio, located
:: via vswhere.exe (a small locator tool VS always installs at this fixed
:: path, regardless of VS version/edition).
if not defined CMAKE_EXE (
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if not exist "!VSWHERE!" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"

    if exist "!VSWHERE!" (
        for /f "usebackq tokens=*" %%I in (`"!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`) do (
            set "CMAKE_EXE=%%I"
        )
    )
)

if not defined CMAKE_EXE (
    echo [ERROR] cmake.exe not found on PATH and no Visual Studio install with CMake could be located.
    echo Open a "Developer Command Prompt for VS" or add CMake to PATH, then retry.
    goto :exit
)

echo Using CMake: !CMAKE_EXE!

:: ---- ensure MSVC compiler environment (cl.exe) is available ----
where cl >nul 2>nul
if errorlevel 1 (
    if exist "!VSWHERE!" (
        for /f "usebackq tokens=*" %%I in (`"!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
            set "VSINSTALL=%%I"
        )
    )
    if defined VSINSTALL (
        if exist "!VSINSTALL!\VC\Auxiliary\Build\vcvarsall.bat" (
            echo Setting up MSVC environment from: !VSINSTALL!
            call "!VSINSTALL!\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
        )
    )
)

echo.
echo === Configuring [%CONFIG%, x64] ===
"!CMAKE_EXE!" -S "%ROOT%." -B "%BUILD_DIR%" -A x64 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
if errorlevel 1 (
    echo [ERROR] CMake configure failed.
    goto :exit
)

echo.
echo === Building [%CONFIG%] ===
"!CMAKE_EXE!" --build "%BUILD_DIR%" --config %CONFIG% -j
if errorlevel 1 (
    echo [ERROR] Build failed.
    goto :exit
)

echo.
echo === Build succeeded ===
set "DLL_PATH=%BUILD_DIR%\%CONFIG%\msys2_ucrt64_shell.dll"
if exist "%DLL_PATH%" (
    echo Output: %DLL_PATH%

    :: ! OPTION B (full package): uncomment when switching to a packed .msix.
    rem if !CONFIG! == Release (
    rem     echo Copying to: %ROOT%\packaging\msys2_ucrt64_shell.dll
    rem     copy /y "%DLL_PATH%" "%ROOT%\packaging\msys2_ucrt64_shell.dll" >nul
    rem )
) else (
    echo [WARN] Expected output not found at: %DLL_PATH%
    echo Check the target name in CMakeLists.txt matches "msys2_ucrt64_shell".
    goto :exit
)

:: ---- update the exit code to success and exit ----
set "EXIT_CODE=0"
goto :exit

:: ---- exit subroutine ----
:exit
if not defined EXIT_CODE set "EXIT_CODE=0"

start "" explorer.exe
endlocal & exit /b %EXIT_CODE%
