@echo off
setlocal 
set rdir=_tmp
set fpath=%~dps0 
call :func %fpath:~0,-1% 
set rdir=_debug
call :func %fpath:~0,-1% 
set rdir=_release
call :func %fpath:~0,-1% 
set rdir=debug
call :func %fpath:~0,-1% 
set rdir=release
call :func %fpath:~0,-1% 
set rdir=build
call :func %fpath:~0,-1% 
set rdir=_UpgradeReport_Files
call :func %fpath:~0,-1% 
goto end 
:func 
for /f "delims=" %%i in ('dir %1 /a:d /b') do IF /I %%i==%rdir% ( rmdir /s /q %1\%%i && echo deleted %1\%%i ) ELSE ( call :func %1\%%i ) 
exit /b 
:end
del /s /f /q Makefile*
del /s /f /q *.pdb
del /s /f /q *.idb
del /s /f /q *.vcproj*
del /s /f /q *.suo
del /s /f /q *.ncb 
del /s /f /q UpgradeLog.XML