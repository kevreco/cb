@echo OFF

set "cb_bat=%~dp0cb.bat"

@REM Recursively iterate over all the "cb.c" files and call "cb.bat" on them
for /R "./tests/" %%f in (cb.c) do (
    if exist "%%f" (
        echo "Starting test for: %%f"
        @REM Execute cb.bat on the current cb.c
        call %cb_bat% --pedantic --file "%%f" || exit /B 1
    )
)

