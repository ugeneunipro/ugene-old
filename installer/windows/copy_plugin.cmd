@echo off

IF NOT EXIST %RELEASE_DIR%\plugins\%1.dll echo "%RELEASE_DIR%\plugins\%1.dll" doesn't exist, skipped 
IF NOT EXIST %RELEASE_DIR%\plugins\%1.dll exit /B 0

echo Copying plugin %1...
copy %RELEASE_DIR%\plugins\%1.dll %OUTPUT_DIR%\plugins
copy %RELEASE_DIR%\plugins\%1.plugin %OUTPUT_DIR%\plugins
copy %RELEASE_DIR%\plugins\%1.license %OUTPUT_DIR%\plugins
copy %RELEASE_DIR%\plugins\%1.map %OUTPUT_DIR%\plugins

dump_syms.exe %RELEASE_DIR%\plugins\%1.dll > %SYMBOLS_DIR%\%1.sym 2>>%DUMP_SYMBOLS_LOG%
@echo on