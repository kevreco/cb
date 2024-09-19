@echo OFF

set "root_dir=%~dp0"
@REM Get the absolute path of the cb.bat file
set "cb_bat=%~dp0cb.bat"

@REM Recursively iterate over all the "cb.c" files and call "cb.bat" on them
for /R "./tests/" %%f in (cb.c) do (
    if exist "%%f" (
		echo "Starting test for: %%f"
		@REM Go to the cb.c directory
		cd %%~dpf
		@REM Execute cb.bat on the current cb.c
		call %cb_bat% || goto fail
    )
)

@REM Restore root directory
cd %root_dir%
exit /B 0

:fail

cd %root_dir%
exit /B 1
