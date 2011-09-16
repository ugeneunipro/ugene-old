PRODUCT_NAME="ugeneui"

VERSION=`cat ../../src/ugene_version.pri | grep UGENE_VERSION | awk -F'=' '{print $2}'`
BUILD_DIR=./ugeneui_bundle
RELEASE_DIR=../../src/_release
TARGET_APP_DIR="$BUILD_DIR/${PRODUCT_NAME}.app/"
TARGET_EXE_DIR="${TARGET_APP_DIR}/Contents/MacOS"
PATH_TO_QT="/usr/local/Trolltech/Qt-4.7.3/lib"

source bundle_common.sh

echo cleaning previous bundle
rm -rf ${BUILD_DIR}
rm -rf ~/.config/Unipro/UGENE*
mkdir $BUILD_DIR


echo
echo copying UGENE bundle 
cp -R $RELEASE_DIR/ugeneui.app/ "$TARGET_APP_DIR"
changeCoreInstallNames ugeneui

mkdir "${TARGET_EXE_DIR}/../Frameworks"
mkdir "${TARGET_EXE_DIR}/plugins"
mkdir "${TARGET_EXE_DIR}/data"

echo copying translations
cp $RELEASE_DIR/transl_en.qm "$TARGET_EXE_DIR"
cp $RELEASE_DIR/transl_ru.qm "$TARGET_EXE_DIR"
cp -R ./qt_menu.nib "${TARGET_EXE_DIR}/../Resources"

echo copying data dir

cp -R "$RELEASE_DIR/../../data" "${TARGET_EXE_DIR}/"
find $TARGET_EXE_DIR -name ".svn" | xargs rm -rf

echo copying qt libraries - plugin dependencies
cp $PATH_TO_QT/libQtOpenGL.4.dylib "${TARGET_EXE_DIR}/../Frameworks/libQtOpenGL.4.dylib"
install_name_tool -id @executable_path/../Frameworks/libQtOpenGL.4.dylib ${TARGET_EXE_DIR}/../Frameworks/libQtOpenGL.4.dylib
changeQtInstallNames ../Frameworks/libQtOpenGL.4.dylib
cp $PATH_TO_QT/libQtSvg.4.dylib "${TARGET_EXE_DIR}/../Frameworks/libQtSvg.4.dylib"
install_name_tool -id @executable_path/../Frameworks/libQtSvg.4.dylib ${TARGET_EXE_DIR}/../Frameworks/libQtSvg.4.dylib
changeQtInstallNames ../Frameworks/libQtSvg.4.dylib
if [ "$1" == "-test" ]
    then
        cp $PATH_TO_QT/libQtTest.4.dylib "${TARGET_EXE_DIR}/../Frameworks/libQtTest.4.dylib"
        install_name_tool -id @executable_path/../Frameworks/libQtTest.4.dylib ${TARGET_EXE_DIR}/../Frameworks/libQtTest.4.dylib
        changeQtInstallNames ../Frameworks/libQtTest.4.dylib
fi


echo Copying core shared libs

add-library U2Algorithm
add-library U2Core
add-library U2Designer
add-library U2Formats
add-library U2Gui
add-library U2Lang
add-library U2Private
add-library U2Remote
add-library U2Test
add-library U2View
add-library ugenedb

echo Copying plugins

add-plugin annotator
add-plugin ball
add-plugin biostruct3d_view
add-plugin bowtie
add-plugin chroma_view
add-plugin circular_view
add-plugin remote_service
add-plugin dna_export
add-plugin dna_graphpack
add-plugin dna_stat
add-plugin dotplot
add-plugin enzymes
add-plugin external_tool_support
add-plugin genome_aligner
add-plugin gor4
add-plugin hmm2
add-plugin hmm3
add-plugin kalign
add-plugin orf_marker
add-plugin phylip
add-plugin primer3
add-plugin psipred
add-plugin query_designer
add-plugin remote_blast
add-plugin repeat_finder
add-plugin sitecon
add-plugin smith_waterman
add-plugin umuscle
add-plugin weight_matrix
add-plugin workflow_designer
add-plugin opencl_support
add-plugin dbi_bam
#add-plugin dbi_file
add-plugin ptools
add-plugin dna_flexibility

if [ "$1" == "-test" ]
   then
      add-plugin CoreTests
      add-plugin test_runner
fi

echo
echo macdeployqt running...
macdeployqt "$TARGET_APP_DIR" 

if [ ! "$1" ] 
   then
      echo
      echo pkg-dmg running...
      pkg-dmg --source $BUILD_DIR --target ugene-${VERSION}-mac-x86-r${BUILD_VCS_NUMBER_new_trunk} --license ../source/LICENSE --volname "Unipro UGENE $VERSION" --symlink /Applications
    
fi


 
