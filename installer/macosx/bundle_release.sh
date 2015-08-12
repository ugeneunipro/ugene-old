PRODUCT_NAME="ugeneui"
PRODUCT_DISPLAY_NAME="Unipro UGENE"

VERSION=`cat ../../src/ugene_version.pri | grep UGENE_VERSION | awk -F'=' '{print $2}'`
ARCHITECTURE=`uname -m`
BUILD_DIR=./release_bundle
RELEASE_DIR=../../src/_release
TARGET_APP_DIR="$BUILD_DIR/${PRODUCT_NAME}.app/"
TARGET_APP_DIR_RENAMED="$BUILD_DIR/${PRODUCT_DISPLAY_NAME}.app/"
TARGET_EXE_DIR="${TARGET_APP_DIR}/Contents/MacOS"
SYMBOLS_DIR=symbols


source bundle_common.sh

echo cleaning previous bundle
rm -rf ${BUILD_DIR}
rm -rf ~/.config/Unipro/UGENE*
mkdir $BUILD_DIR

echo Preparing debug symbols location
rm -rf ${SYMBOLS_DIR}
rm -f "${SYMBOLS_DIR}.tar.gz"
mkdir "${SYMBOLS_DIR}"

echo
echo copying UGENE bundle 
cp -R $RELEASE_DIR/ugeneui.app/ "$TARGET_APP_DIR"

echo copying icons
cp ../../src/ugeneui/images/ugene-doc.icns "$TARGET_APP_DIR/Contents/Resources"

mkdir "${TARGET_EXE_DIR}/../Frameworks"
mkdir "${TARGET_EXE_DIR}/plugins"
mkdir "${TARGET_EXE_DIR}/data"

echo copying translations
cp $RELEASE_DIR/transl_*.qm "$TARGET_EXE_DIR"
cp -R ./qt_menu.nib "${TARGET_EXE_DIR}/../Resources"
find "${TARGET_EXE_DIR}/../Resources/qt_menu.nib" -name ".svn" | xargs rm -rf

echo copying data dir

cp -R "$RELEASE_DIR/../../data" "${TARGET_EXE_DIR}/"
find $TARGET_EXE_DIR -name ".svn" | xargs rm -rf

#include external tools package if applicable
if [ -e "$RELEASE_DIR/../../tools" ]; then
    cp -R "$RELEASE_DIR/../../tools" "${TARGET_EXE_DIR}/"
    find $TARGET_EXE_DIR -name ".svn" | xargs rm -rf
fi

echo Copying UGENE binaries
add-binary ugeneui
add-binary ugenem
add-binary ugenecl
add-binary plugins_checker
cp ./ugene "$TARGET_EXE_DIR"

echo Copying core shared libs

add-library U2Algorithm
add-library U2Core
add-library U2Designer
add-library U2Formats
add-library U2Gui
add-library U2Lang
add-library U2Private
add-library U2Remote
add-library U2Script
add-library U2Test
add-library U2View
add-library ugenedb
add-library breakpad
if [ "$1" == "-test" ]
   then
      add-library gtest
fi

echo Copying plugins

# plugins to copy to the bundle
# to ignore plugin remove it
PLUGIN_LIST="annotator \
            ball \
            biostruct3d_view \
            browser_support \
            chroma_view \
            circular_view \
            dbi_bam \
            dna_export \
            dna_flexibility \
            dna_graphpack \
            dna_stat \
            dotplot \
            enzymes \
            expert_discovery \
            external_tool_support \
            genome_aligner \
            gor4 \
            hmm2 \
            hmm3 \
            kalign \
            linkdata_support \
            opencl_support \
            orf_marker \
            pcr \
            phylip \
            primer3 \
            psipred \
            ptools \
            query_designer \
            remote_blast \
            remote_service \
            repeat_finder \
            sitecon \
            smith_waterman \
            umuscle \
            variants \
            weight_matrix \
            workflow_designer"

if [ "$1" == "-test" ]
   then
   PLUGIN_LIST="$PLUGIN_LIST CoreTests \
                             GUITestBase \
                             perf_monitor \
                             test_runner \
                             api_tests"
fi

for PLUGIN in $PLUGIN_LIST
do
    add-plugin $PLUGIN
done

echo
echo macdeployqt running...
macdeployqt "$TARGET_APP_DIR" -no-strip -executable="$TARGET_EXE_DIR"/ugenecl -executable="$TARGET_EXE_DIR"/ugenem -executable="$TARGET_EXE_DIR"/plugins_checker

# Do not use @loader_path that produced by macdeployqt with "-executable" argument,
# it cause a crash with plugins loading (UGENE-2994)
# Restore @executable_path:
echo
echo @executable_path restoring...
for PLUGIN in $PLUGIN_LIST
do
    restorePluginsQtInstallNames $PLUGIN
done

mv "$TARGET_APP_DIR" "$TARGET_APP_DIR_RENAMED"

cd  $BUILD_DIR 
ln -s ./Unipro\ UGENE.app/Contents/MacOS/data/samples ./Samples
cd ..

echo copy readme.txt file
cp ./readme.txt $BUILD_DIR/readme.txt

if [ ! "$1" ]; then
    echo
    echo Compressing symbols...
    tar czf "${SYMBOLS_DIR}.tar.gz" "${SYMBOLS_DIR}"

    echo
    echo pkg-dmg running...
    ./pkg-dmg --source $BUILD_DIR --target ugene-${VERSION}-mac-${ARCHITECTURE}-r${BUILD_VCS_NUMBER_new_trunk} --license ./LICENSE.with_3rd_party --volname "Unipro UGENE $VERSION" --symlink /Applications
fi
