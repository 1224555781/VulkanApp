@echo off
setlocal

echo ===== Checking BuildRealEngine compilation status =====

set CS_FILE=buildTool\BuildRealEngine.cs
set EXE_FILE=buildTool\BuildRealEngine.exe
set CONFIG_FILE=buildTool\BuildConfig.cs

:: Check if executable exists or is older than source files
if not exist "%EXE_FILE%" (
    goto :compile
) else (
    for %%i in ("%CS_FILE%") do set CS_DATE=%%~ti
    for %%i in ("%CONFIG_FILE%") do set CONFIG_DATE=%%~ti
    for %%i in ("%EXE_FILE%") do set EXE_DATE=%%~ti
    if "%CS_DATE%" gtr "%EXE_DATE%" goto :compile
    if "%CONFIG_DATE%" gtr "%EXE_DATE%" goto :compile
)

goto :run

:compile
echo Finding C# compiler...

:: Try using dotnet CLI first
where dotnet >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Compiling with dotnet CLI...
    dotnet new console -n BuilderTemp -o buildTool\BuilderTemp --force
    copy %CS_FILE% buildTool\BuilderTemp\Program.cs >nul
    copy %CONFIG_FILE% buildTool\BuilderTemp\BuildConfig.cs >nul
    dotnet build buildTool\BuilderTemp -o buildTool
    if exist buildTool\BuilderTemp.dll (
        move /y buildTool\BuilderTemp.dll %EXE_FILE% >nul
        goto :cleanup
    ) else if exist buildTool\BuilderTemp.exe (
        move /y buildTool\BuilderTemp.exe %EXE_FILE% >nul
        goto :cleanup
    )
    goto :try_csc
)

:try_csc
:: Try using .NET Framework csc.exe
set CSC_PATH=
for %%v in (4.6.2 3.5 2.0) do (
    for %%p in ("%SystemRoot%\Microsoft.NET\Framework64\v%%v*" "%SystemRoot%\Microsoft.NET\Framework\v%%v*") do (
        if exist "%%p\csc.exe" (
            set CSC_PATH="%%p\csc.exe"
            goto :found_csc
        )
    )
)

:found_csc
if not defined CSC_PATH (
    echo C# compiler not found!
    echo Please install .NET Framework or .NET Core SDK.
    exit /b 1
)

echo Compiling with %CSC_PATH%...
%CSC_PATH% /out:%EXE_FILE% /target:exe /main:BuildRealEngine /reference:System.IO.Compression.FileSystem.dll %CS_FILE% %CONFIG_FILE%
if errorlevel 1 (
    goto :compile_failed
)
echo Compilation successful.
goto :run

:compile_failed
echo Compilation failed.
exit /b 1

:cleanup
if exist buildTool\BuilderTemp (
    rd /s /q buildTool\BuilderTemp
)

:run
echo Running BuildRealEngine...
pushd buildTool
BuildRealEngine.exe %*
popd
echo ===== Generation process completed =====
