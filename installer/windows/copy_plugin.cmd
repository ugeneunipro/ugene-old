copy %RELEASE_DIR%\plugins\%1.dll %OUTPUT_DIR%\plugins
copy %RELEASE_DIR%\plugins\%1.plugin %OUTPUT_DIR%\plugins
copy %RELEASE_DIR%\plugins\%1.license %OUTPUT_DIR%\plugins
copy %RELEASE_DIR%\plugins\%1.map %OUTPUT_DIR%\plugins

%INSTALL_DIR%/dump_syms %RELEASE_DIR%\plugins\%1.dll > %SYMBOLS_DIR%/%1.dll.sym
