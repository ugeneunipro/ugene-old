REM the root dir for this scirpt is svn trunk
set U_ROOT=%cd%
set U_VERSION=%1
set OUTPUT_DIR=%U_ROOT%\installer\windows_x64\ugene-%U_VERSION%
set INSTALL_DIR=%U_ROOT%\installer\windows_x64
set BINARY_ZIPFILE=%INSTALL_DIR%\ugene-%U_VERSION%-win-x64-r%BUILD_VCS_NUMBER_new_trunk%.zip


IF EXIST %OUTPUT_DIR% del /F /S /Q %OUTPUT_DIR%
IF EXIST %BINARY_ZIPFILE% del /F /Q %BINARY_ZIPFILE%
set RELEASE_DIR=src\_release

REM create release dir
mkdir %OUTPUT_DIR%

REM copy includes
xcopy /S %INSTALL_DIR%\includes\* %OUTPUT_DIR%

REM copy executables
copy %RELEASE_DIR%\ugeneui.exe %OUTPUT_DIR%
copy %RELEASE_DIR%\ugenecl.exe %OUTPUT_DIR%
copy %RELEASE_DIR%\ugenem.exe %OUTPUT_DIR%
copy %RELEASE_DIR%\ugeneui.map %OUTPUT_DIR%
copy %RELEASE_DIR%\ugenecl.map %OUTPUT_DIR%
echo. > %OUTPUT_DIR%\UGENE.ini

REM copy translations
copy %RELEASE_DIR%\transl_en.qm %OUTPUT_DIR%
copy %RELEASE_DIR%\transl_ru.qm %OUTPUT_DIR%
copy %RELEASE_DIR%\transl_cs.qm %OUTPUT_DIR%
copy %RELEASE_DIR%\transl_zh.qm %OUTPUT_DIR%

REM copy data
svn export %U_ROOT%\data %OUTPUT_DIR%\data
copy %U_ROOT%\data\manuals\*.pdf %OUTPUT_DIR%\data\manuals\

REM copy libs 
copy %RELEASE_DIR%\ugenedb.dll %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Algorithm.dll %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Core.dll %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Designer.dll  %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Formats.dll  %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Gui.dll  %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Lang.dll  %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Private.dll  %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Remote.dll  %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Test.dll  %OUTPUT_DIR%
copy %RELEASE_DIR%\U2View.dll  %OUTPUT_DIR%
copy %RELEASE_DIR%\ugenedb.map %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Algorithm.map %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Core.map %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Designer.map  %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Formats.map  %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Gui.map  %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Lang.map  %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Private.map  %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Remote.map  %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Test.map  %OUTPUT_DIR%
copy %RELEASE_DIR%\U2View.map  %OUTPUT_DIR%

REM copy plugins
call %INSTALL_DIR%/copy_plugin.cmd annotator
call %INSTALL_DIR%/copy_plugin.cmd ball
call %INSTALL_DIR%/copy_plugin.cmd biostruct3d_view
call %INSTALL_DIR%/copy_plugin.cmd chroma_view
call %INSTALL_DIR%/copy_plugin.cmd circular_view
call %INSTALL_DIR%/copy_plugin.cmd cuda_support
call %INSTALL_DIR%/copy_plugin.cmd dna_export
call %INSTALL_DIR%/copy_plugin.cmd dna_graphpack
call %INSTALL_DIR%/copy_plugin.cmd dna_stat
call %INSTALL_DIR%/copy_plugin.cmd dotplot
call %INSTALL_DIR%/copy_plugin.cmd enzymes
call %INSTALL_DIR%/copy_plugin.cmd external_tool_support
call %INSTALL_DIR%/copy_plugin.cmd genome_aligner
call %INSTALL_DIR%/copy_plugin.cmd gor4
call %INSTALL_DIR%/copy_plugin.cmd hmm2
call %INSTALL_DIR%/copy_plugin.cmd hmm3
call %INSTALL_DIR%/copy_plugin.cmd kalign
call %INSTALL_DIR%/copy_plugin.cmd opencl_support
call %INSTALL_DIR%/copy_plugin.cmd orf_marker
call %INSTALL_DIR%/copy_plugin.cmd phylip
call %INSTALL_DIR%/copy_plugin.cmd primer3
call %INSTALL_DIR%/copy_plugin.cmd psipred
call %INSTALL_DIR%/copy_plugin.cmd query_designer
call %INSTALL_DIR%/copy_plugin.cmd remote_blast
call %INSTALL_DIR%/copy_plugin.cmd remote_service
call %INSTALL_DIR%/copy_plugin.cmd repeat_finder
call %INSTALL_DIR%/copy_plugin.cmd sitecon
call %INSTALL_DIR%/copy_plugin.cmd smith_waterman
REM call %INSTALL_DIR%/copy_plugin.cmd umuscle
call %INSTALL_DIR%/copy_plugin.cmd weight_matrix
call %INSTALL_DIR%/copy_plugin.cmd workflow_designer
call %INSTALL_DIR%/copy_plugin.cmd dbi_sqlite
call %INSTALL_DIR%/copy_plugin.cmd dbi_bam
call %INSTALL_DIR%/copy_plugin.cmd assembly_browser
call %INSTALL_DIR%/copy_plugin.cmd expert_discovery
call %INSTALL_DIR%/copy_plugin.cmd ptools
call %INSTALL_DIR%/copy_plugin.cmd dna_flexibility
call %INSTALL_DIR%/copy_plugin.cmd variants


cd %INSTALL_DIR%
zip -r %BINARY_ZIPFILE% ugene-%U_VERSION%\*
cd %U_ROOT%




