# this script's home dir is trunk/installer/linux/X11

PRODUCT_NAME="ugene"
VERSION=`cat ../../../src/ugene_version.pri | grep UGENE_VERSION | awk -F'=' '{print $2}'`
RELEASE_DIR=../../../src/_release
DATA_DIR=../../../data
TARGET_APP_DIR="${PRODUCT_NAME}-${VERSION}"

#PATH_TO_QT_LIBS="$HOME/qtsdk-2010.01/qt/lib"

if [ -z "$PATH_TO_QT_LIBS" ]; then 
   echo PATH_TO_QT_LIBS environment variable is not set!
   exit -1
fi

echo cleaning previous bundle
rm -rf ${TARGET_APP_DIR}
rm -rf *.tar.gz
mkdir $TARGET_APP_DIR


echo
echo copying ugenecl
cp -v $RELEASE_DIR/ugenecl "$TARGET_APP_DIR"

echo copying ugeneui
cp -v $RELEASE_DIR/ugeneui "$TARGET_APP_DIR"

echo copying ugene startup script
cp -v $RELEASE_DIR/ugenem "$TARGET_APP_DIR"

echo copying ugene startup script
cp -v $RELEASE_DIR/ugene "$TARGET_APP_DIR"

echo copying man page for UGENE
mkdir -v "$TARGET_APP_DIR/man1"
cp -v ../../_common_data/ugene.1.gz "$TARGET_APP_DIR/man1"

echo copying README file
cp -v ../../_common_data/README "$TARGET_APP_DIR"

echo copying LICENSE file
cp -v ../../source/LICENSE "$TARGET_APP_DIR"


mkdir "${TARGET_APP_DIR}/plugins"

echo copying translations
cp -v $RELEASE_DIR/transl_en.qm "$TARGET_APP_DIR"
cp -v $RELEASE_DIR/transl_ru.qm "$TARGET_APP_DIR"
cp -v $RELEASE_DIR/transl_cs.qm "$TARGET_APP_DIR"
cp -v $RELEASE_DIR/transl_cn.qm "$TARGET_APP_DIR"


echo copying data dir
cp -R "$RELEASE_DIR/../../data"  "${TARGET_APP_DIR}"

echo
echo copying core shared libs
cp -v "$RELEASE_DIR/libU2Algorithm.so.1" "${TARGET_APP_DIR}"
cp -v "$RELEASE_DIR/libU2Core.so.1" "${TARGET_APP_DIR}"
cp -v "$RELEASE_DIR/libU2Designer.so.1" "${TARGET_APP_DIR}"
cp -v "$RELEASE_DIR/libU2Formats.so.1" "${TARGET_APP_DIR}"
cp -v "$RELEASE_DIR/libU2Gui.so.1" "${TARGET_APP_DIR}"
cp -v "$RELEASE_DIR/libU2Lang.so.1" "${TARGET_APP_DIR}"
cp -v "$RELEASE_DIR/libU2Private.so.1" "${TARGET_APP_DIR}"
cp -v "$RELEASE_DIR/libU2Remote.so.1" "${TARGET_APP_DIR}"
cp -v "$RELEASE_DIR/libU2Test.so.1" "${TARGET_APP_DIR}"
cp -v "$RELEASE_DIR/libU2View.so.1" "${TARGET_APP_DIR}"
cp -v "$RELEASE_DIR/libugenedb.so.1" "${TARGET_APP_DIR}"

echo
echo copying qt libraries
cp -v "$PATH_TO_QT_LIBS/libQtCore.so.4" "${TARGET_APP_DIR}"
strip -v "${TARGET_APP_DIR}/libQtCore.so.4"
cp -v "$PATH_TO_QT_LIBS/libQtGui.so.4" "${TARGET_APP_DIR}"
strip -v "${TARGET_APP_DIR}/libQtGui.so.4"
cp -v "$PATH_TO_QT_LIBS/libQtXml.so.4" "${TARGET_APP_DIR}"
strip -v "${TARGET_APP_DIR}/libQtXml.so.4"
cp -v "$PATH_TO_QT_LIBS/libQtXmlPatterns.so.4" "${TARGET_APP_DIR}"
strip -v "${TARGET_APP_DIR}/libQtXmlPatterns.so.4"
cp -v "$PATH_TO_QT_LIBS/libQtScript.so.4" "${TARGET_APP_DIR}"
strip -v "${TARGET_APP_DIR}/libQtScript.so.4"
cp -v "$PATH_TO_QT_LIBS/libQtWebKit.so.4" "${TARGET_APP_DIR}"
strip -v "${TARGET_APP_DIR}/libQtWebKit.so.4"
cp -v "$PATH_TO_QT_LIBS/libQtNetwork.so.4" "${TARGET_APP_DIR}"
strip -v "${TARGET_APP_DIR}/libQtNetwork.so.4"
cp -v "$PATH_TO_QT_LIBS/libQtOpenGL.so.4" "${TARGET_APP_DIR}"
strip -v "${TARGET_APP_DIR}/libQtOpenGL.so.4"
cp -v "$PATH_TO_QT_LIBS/libQtSvg.so.4" "${TARGET_APP_DIR}"
strip -v "${TARGET_APP_DIR}/libQtSvg.so.4"


if [ "$1" == "-test" ]
    then
        cp "$PATH_TO_QT_LIBS/libQtTest.so.4" "${TARGET_APP_DIR}"
fi

function add-plugin {
    plugin=$1
    echo "Registering plugin: ${plugin}"

    PLUGIN_LIB="lib${plugin}.so"
    PLUGIN_DESC="${plugin}.plugin"
    
    echo $PLUGIN_LIB
    echo

    if [ ! -f ${RELEASE_DIR}/plugins/${PLUGIN_LIB} ] ;  
    then  
        echo "Plugin library file not found: ${PLUGIN_LIB} !"
        exit 1
    fi

    if [ ! -f ${RELEASE_DIR}/plugins/${PLUGIN_DESC} ] ; 
    then
        echo "Plugin descriptor file not found: ${PLUGIN_DESC} !"
        exit 1
    fi
   
    cp -v "${RELEASE_DIR}/plugins/${PLUGIN_LIB}"  "${TARGET_APP_DIR}/plugins/"
    cp -v "${RELEASE_DIR}/plugins/${PLUGIN_DESC}" "${TARGET_APP_DIR}/plugins/"
}

echo copying plugins
add-plugin annotator
add-plugin ball
add-plugin biostruct3d_view
add-plugin chroma_view
add-plugin circular_view
add-plugin dna_export
add-plugin dna_graphpack
add-plugin dna_stat
add-plugin enzymes
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
add-plugin workflow_designer
add-plugin genome_aligner
add-plugin weight_matrix
add-plugin dotplot
add-plugin remote_service
add-plugin external_tool_support
add-plugin dbi_sqlite
add-plugin dbi_bam
add-plugin assembly_browser
add-plugin ptools
add-plugin dna_flexibility

if [ "$1" == "-test" ]; then
  add-plugin test_runner
fi

# remove svn dirs
find $TARGET_APP_DIR -name ".svn" | xargs rm -rf 

#download and include external tools package if applicable
if [ -z "$EXT_TOOLS_URL" ]; then 
   echo
   echo EXT_TOOLS_URL environment variable is not set: skipping this step
   echo
else
   pushd $TARGET_APP_DIR 
   wget $EXT_TOOLS_URL
   EXT_TOOLS_PACKAGE=`basename $EXT_TOOLS_URL` 
   tar -xf $EXT_TOOLS_PACKAGE
   rm $EXT_TOOLS_PACKAGE
   popd
fi


REVISION=$BUILD_VCS_NUMBER_new_trunk
#`svn status -u | sed -n -e '/revision/p' | awk '{print $4}'`
DATE=`date '+%d_%m_%H-%M'`
ARCH=`uname -m`
if [ "$1" == "-test" ]; then
   TEST="-test"
fi

PACKAGE_NAME=$PRODUCT_NAME"-"$VERSION"-linux-"$ARCH"-r"$REVISION$TEST

tar -cf $PACKAGE_NAME.tar $TARGET_APP_DIR/
gzip -v $PACKAGE_NAME.tar

 
