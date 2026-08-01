@echo off

for %%A in (%*) do (
    IF /I "%%~A" == "release" (
        set build_dir=release
        set build_opt=RELEASE
        goto :found
    )
    IF /I "%%~A" == "rebuild" (
        make clean
        rm compile_commands.json
    )
) 

set build_dir=debug
set build_opt=DEBUG

:found
clear
echo ----- BUILDING -----
echo:

echo Building to %build_opt%
echo:
compiledb -- make BUILD=%build_opt%

echo ----- BUILD FINISHED -----
echo: 

start "" /D "build\%build_dir%\bin\" "sandbox.exe"
