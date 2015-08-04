copy %RELEASE_DIR%\%1.exe %OUTPUT_DIR%
copy %RELEASE_DIR%\%1.map %OUTPUT_DIR%

%INSTALL_DIR%/dump_syms %RELEASE_DIR%\%1.dll > %SYMBOLS_DIR%/%1.exe.sym
