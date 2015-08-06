copy %RELEASE_DIR%\%1.dll %OUTPUT_DIR%
copy %RELEASE_DIR%\%1.map %OUTPUT_DIR%

dump_syms.exe %RELEASE_DIR%\%1.dll > %SYMBOLS_DIR%\%1.sym 2>>%DUMP_SYMBOLS_LOG%