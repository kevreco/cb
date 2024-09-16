@echo OFF

set "root_dir=%~dp0"
@REM Get the absolute path of the cb.bat file
set "cb_bat=%~dp0cb.bat"

@REM Recursively iterate over all the "cb.c" files and call "cb.bat" on them
for /R "./" %%f in (cb.c) do (
    if exist "%%f" (
		@REM Go to the cb.c directory
		cd %%~dpf
		@REM Execute cb.bat on the current cb.c
		call %cb_bat%
    )
)

@REM Restore root directory
cd %root_dir%