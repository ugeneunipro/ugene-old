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
mkdir "%OUTPUT_DIR%\plugins"

REM copy includes
xcopy /E %INSTALL_DIR%\includes\* %OUTPUT_DIR%
REM copy Qt libraries
copy %PATH_TO_QT_LIBS%\Qt5Core.dll %OUTPUT_DIR%
copy %PATH_TO_QT_LIBS%\Qt5Gui.dll %OUTPUT_DIR%
copy %PATH_TO_QT_LIBS%\Qt5Multimedia.dll %OUTPUT_DIR%
copy %PATH_TO_QT_LIBS%\Qt5MultimediaWidgets.dll %OUTPUT_DIR%
copy %PATH_TO_QT_LIBS%\Qt5Network.dll %OUTPUT_DIR%
copy %PATH_TO_QT_LIBS%\Qt5OpenGL.dll %OUTPUT_DIR%
copy %PATH_TO_QT_LIBS%\Qt5Positioning.dll %OUTPUT_DIR%
copy %PATH_TO_QT_LIBS%\Qt5PrintSupport.dll %OUTPUT_DIR%
copy %PATH_TO_QT_LIBS%\Qt5Qml.dll %OUTPUT_DIR%
copy %PATH_TO_QT_LIBS%\Qt5Quick.dll %OUTPUT_DIR%
copy %PATH_TO_QT_LIBS%\Qt5Script.dll %OUTPUT_DIR%
copy %PATH_TO_QT_LIBS%\Qt5Sensors.dll %OUTPUT_DIR%
copy %PATH_TO_QT_LIBS%\Qt5Sql.dll %OUTPUT_DIR%
copy %PATH_TO_QT_LIBS%\Qt5Svg.dll %OUTPUT_DIR%
copy %PATH_TO_QT_LIBS%\Qt5WebChannel.dll %OUTPUT_DIR%
copy %PATH_TO_QT_LIBS%\Qt5WebKit.dll %OUTPUT_DIR%
copy %PATH_TO_QT_LIBS%\Qt5WebKitWidgets.dll %OUTPUT_DIR%
copy %PATH_TO_QT_LIBS%\Qt5Widgets.dll %OUTPUT_DIR%
copy %PATH_TO_QT_LIBS%\Qt5Xml.dll %OUTPUT_DIR%
xcopy /E %PATH_TO_QT_LIBS%\icu*.dll %OUTPUT_DIR%
mkdir "%OUTPUT_DIR%\sqldrivers"
copy %PATH_TO_QT_LIBS%\..\plugins\sqldrivers\qsqlmysql.dll %OUTPUT_DIR%\sqldrivers\
mkdir "%OUTPUT_DIR%\imageformats"
copy %PATH_TO_QT_LIBS%\..\plugins\imageformats\qgif.dll %OUTPUT_DIR%\imageformats\
copy %PATH_TO_QT_LIBS%\..\plugins\imageformats\qjpeg.dll %OUTPUT_DIR%\imageformats\
copy %PATH_TO_QT_LIBS%\..\plugins\imageformats\qsvg.dll %OUTPUT_DIR%\imageformats\
copy %PATH_TO_QT_LIBS%\..\plugins\imageformats\qtiff.dll %OUTPUT_DIR%\imageformats\

REM copy external tools if exists
xcopy /E %RELEASE_DIR%\tools\* %OUTPUT_DIR%\tools\
if DEFINED UGENE_R_DIST_PATH (
    xcopy /E %UGENE_R_DIST_PATH% %OUTPUT_DIR%\tools\
)

REM copy executables
copy %RELEASE_DIR%\ugeneui.exe %OUTPUT_DIR%
copy %RELEASE_DIR%\ugenecl.exe %OUTPUT_DIR%
copy %RELEASE_DIR%\ugenem.exe %OUTPUT_DIR%
copy %RELEASE_DIR%\plugins_checker.exe %OUTPUT_DIR%
copy %RELEASE_DIR%\ugeneui.map %OUTPUT_DIR%
copy %RELEASE_DIR%\ugenecl.map %OUTPUT_DIR%
echo. > %OUTPUT_DIR%\UGENE.ini

REM copy translations
copy %RELEASE_DIR%\transl_en.qm %OUTPUT_DIR%
copy %RELEASE_DIR%\transl_ru.qm %OUTPUT_DIR%
copy %RELEASE_DIR%\transl_cs.qm %OUTPUT_DIR%
copy %RELEASE_DIR%\transl_zh.qm %OUTPUT_DIR%

REM copy data
xcopy /I /S %U_ROOT%\data %OUTPUT_DIR%\data
copy %U_ROOT%\data\manuals\*.pdf %OUTPUT_DIR%\data\manuals\
if DEFINED UGENE_CISTROME_PATH (
    move %UGENE_CISTROME_PATH% %OUTPUT_DIR%\data\
)

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
copy %RELEASE_DIR%\U2Script.dll  %OUTPUT_DIR%
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
copy %RELEASE_DIR%\U2Script.map  %OUTPUT_DIR%
copy %RELEASE_DIR%\U2Test.map  %OUTPUT_DIR%
copy %RELEASE_DIR%\U2View.map  %OUTPUT_DIR%

REM copy plugins
call %INSTALL_DIR%/copy_plugin.cmd annotator
call %INSTALL_DIR%/copy_plugin.cmd ball
call %INSTALL_DIR%/copy_plugin.cmd biostruct3d_view
call %INSTALL_DIR%/copy_plugin.cmd browser_support
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
call %INSTALL_DIR%/copy_plugin.cmd umuscle
call %INSTALL_DIR%/copy_plugin.cmd weight_matrix
call %INSTALL_DIR%/copy_plugin.cmd workflow_designer
call %INSTALL_DIR%/copy_plugin.cmd dbi_sqlite
call %INSTALL_DIR%/copy_plugin.cmd dbi_bam
call %INSTALL_DIR%/copy_plugin.cmd assembly_browser
call %INSTALL_DIR%/copy_plugin.cmd expert_discovery
call %INSTALL_DIR%/copy_plugin.cmd pcr
call %INSTALL_DIR%/copy_plugin.cmd ptools
call %INSTALL_DIR%/copy_plugin.cmd dna_flexibility
call %INSTALL_DIR%/copy_plugin.cmd variants


cd %INSTALL_DIR%
if NOT DEFINED UGENE_CISTROME_PATH (
zip -r %BINARY_ZIPFILE% ugene-%U_VERSION%\*
)

if DEFINED UGENE_CISTROME_PATH (
    7z a -r %BINARY_ZIPFILE% ugene-%U_VERSION%/*
    move %OUTPUT_DIR%\data\cistrome %UGENE_CISTROME_PATH%
)
cd %U_ROOT%




