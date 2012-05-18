@ECHO OFF
echo "##teamcity[blockOpened name='Conf build']"
set U_ROOT=%cd%
IF NOT DEFINED U_NAME (set U_NAME=ugene)
IF NOT DEFINED U_VERSION (set U_VERSION=1_5_2)
set U_FULLNAME=%U_NAME%_%U_VERSION%
IF NOT DEFINED TARGET_ARCH (set TARGET_ARCH=x86)
REM TODO: extract the values above.
set UGENE_PRINT_TO_CONSOLE=true
set UGENE_USE_NATIVE_DIALOGS=false
IF NOT DEFINED U_BTYPE (set U_BTYPE=Release)
IF "%U_BTYPE%"=="Debug" (
  set UGENE_INI=UGENED.ini
  set UGENE_EXE=ugened.exe
  set TEST_RUNNER_DLL=test_runnerd.dll
  set BIN_DIR_NAME=_debug
) ELSE (
  set UGENE_INI=UGENE.ini
  set UGENE_EXE=ugene.exe
  set TEST_RUNNER_DLL=test_runner.dll
  set BIN_DIR_NAME=_release
)
REM IF NOT DEFINED

REM set PATH=%PATH%;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;C:\tools\Qt\4.5.2\bin\;
set Path=%Path%;C:/Program Files/NSIS
call "C:\Program Files\Microsoft Visual Studio 9.0\Common7\Tools\vsvars32.bat"
set
echo "##teamcity[blockClosed name='Conf build']"



IF NOT "%UGENE_DISTR_SKIP_BUILD%"==1 (
echo ##teamcity[blockOpened name='qmake msvc project']
echo "cd %U_ROOT%\src"
cd %U_ROOT%\src
echo "call qmake_msvc_proj.cmd"
call qmake_msvc_proj.cmd 
echo ##teamcity[blockClosed name='qmake msvc project']


echo ##teamcity[blockOpened name='Build Solution ugene.sln']
echo "cd %U_ROOT%"
cd %U_ROOT%
echo "devenv.exe ugene.sln /build "%U_BTYPE%^|Win32"
devenv.exe ugene.sln /build "%U_BTYPE%|Win32" /out "build.log"
echo "Checking errorlevel status ..."
IF ERRORLEVEL 1 GOTO END_WITH_COMPL_ERR
echo ##teamcity[blockClosed name='Build Solution ugene.sln']


echo "##teamcity[blockOpened name='Building distr']"
echo "cd %U_ROOT%\installer\windows"
cd %U_ROOT%\installer\windows
echo "makensis ugene.nsh"
makensis ugene.nsh
IF ERRORLEVEL 1 GOTO END_WITH_COMPL_ERR
echo "##teamcity[blockClosed name='Building distr']"
)



IF NOT "%UGENE_DISTR_SKIP_INSTALL%"==1 (
echo "##teamcity[blockOpened name='Installing distr']"
echo "IF EXIST "%PROGRAMFILES%\Unipro uGene\Uninst.exe" ("%PROGRAMFILES%\Unipro uGene\Uninst.exe" /S)"
IF EXIST "%PROGRAMFILES%\Unipro uGene\Uninst.exe" (%PROGRAMFILES%\Unipro uGene\Uninst.exe /S)
echo "build\%U_FULLNAME%_win_%TARGET_ARCH%.exe /S /D C:/Program Files/Unipro uGENE"
build\%U_FULLNAME%_win_%TARGET_ARCH%.exe /S /D C:/Program Files/Unipro uGENE
echo "##teamcity[blockOpened name='Installing distr']"
)


IF NOT "%UGENE_DISTR_SKIP_TESTRUNNER%"==1 (
echo "##teamcity[blockOpened name='Installing %TEST_RUNNER_DLL%']"
echo "copy ..\..\src\%BIN_DIR_NAME%\plugins\%TEST_RUNNER_DLL% "%PROGRAMFILES%\Unipro uGENE\plugins""
copy ..\..\src\%BIN_DIR_NAME%\plugins\%TEST_RUNNER_DLL% "%PROGRAMFILES%\Unipro uGENE\plugins"
echo "copy ..\..\scripts\libTestHelper.js "%PROGRAMFILES%\Unipro uGENE\scripts"
copy ..\..\scripts\libTestHelper.js "%PROGRAMFILES%\Unipro uGENE\scripts"
echo "##teamcity[blockClosed name='Installing %TEST_RUNNER_DLL%']"
)


echo "##teamcity[blockOpened name='Running tests']"
echo "set Path=%Path%;C:/Program Files/Unipro uGENE"
set Path=%Path%;C:/Program Files/Unipro uGENE
echo "set TEST_PATH=%U_ROOT%\test\"
set TEST_PATH=%U_ROOT%\test\
IF NOT DEFINED TEST_SUITE ( set TEST_SUITE=nightlyExcludeTests.list)
IF EXIST ../test/test_report.html del /F ../test/test_report.html
IF EXIST "%USERPROFILE%\Application Data\Unipro\%UGENE_INI%" del /F "%USERPROFILE%\Application Data\Unipro\%UGENE_INI%"
IF EXIST "C:\Documents and Settings\NetworkService\Application Data\Unipro\%UGENE_INI%" del /F "C:\Documents and Settings\NetworkService\Application Data\Unipro\%UGENE_INI%"
IF EXIST "C:\Documents and Settings\LocalService\Application Data\Unipro\%UGENE_INI%" del /F "C:\Documents and Settings\LocalService\Application Data\Unipro\%UGENE_INI%"
REM cd ..\..\src\_release
echo "%UGENE_EXE% ../../test/gui/component/plugins/TestRunner/run_suites.js"
%UGENE_EXE% ../../test/gui/component/plugins/TestRunner/run_suites.js
IF ERRORLEVEL 1 GOTO END_WITH_RUN_ERR

IF EXIST ../../test/test_report.html del /F ../../test/test_report.html
echo ##teamcity[blockClosed name='Runing Tests'] 

exit 0

:END_WITH_COMPL_ERR
echo ##teamcity[buildStatus status='FAILURE' text='{build.status.text}. Solution building failed.'] 
exit 0
:END_WITH_RUN_ERR
echo ##teamcity[buildStatus status='SUCCESS' text='{build.status.text}. Testing failed.'] 
exit 0
