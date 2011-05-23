U_NAME=ugene
U_VERSION=`cat ../../src/ugene_version.pri | grep UGENE_VERSION | awk -F'=' '{print $2}'`
U_REVISION=`svn status -u | sed -n -e '/revision/p' | awk '{print $4}'`
DST_NAME=$U_NAME-$U_VERSION

#Files to include into distribution bundle

#warning!   if in SRC_FILES file path(no directory) before file name must be stay ./ ;for example: "../.././ugene"
SRC_FILES=(
    "./COPYRIGHT"
    "./LICENSE"
    "../.././ugene.pro"
    "../../src/gen_bin_script.cmd"
    "../../src/ugene_globals.pri"
    "../../src/ugene_plugin_common.pri"
    "../../src/ugene_plugin_descriptor.pri"
    "../../src/ugene_lib_common.pri"
    "../../src/ugene_version.pri"
    "../.././build.txt"
    "../../src/libs_3rdparty"
    "../../src/plugins_3rdparty"
    "../../src/plugins"
    "../../installer/_common_data"
    "../../installer/linux/fedora"
    "../../installer/linux/ubuntu"
    "../../installer/macosx"
    "../../installer/windows/images"
    "../../installer/windows/ugene.nsh"
    "../../installer/windows/ugene_extensions.nsh"
    "../../src/corelibs/"
    "../../src/include/"
    "../../src/ugeneui/"
    "../../src/ugenecl/"
    "../../src/ugenem/"
    "../../data/"
)

DST_DIR=./$DST_NAME/

if [[ "$1" == "bzip2" ]]
then
    DST_FILE="$DST_NAME-src-r$U_REVISION.tar.bz2"
    TAR_ARG='pvcjf'
else
    DST_FILE="$DST_NAME-src-r$U_REVISION.tar.gz"
    TAR_ARG='pvczf'
fi

EXCLUDE_LIST=(
    "*.svn"
    "*.vcproj"
    "*_tmp*"
    "*vcproj*"
    "*.pdb"
    "*.idb"
    "*Makefile*"
    "*.user"
    "../../src/plugins/atistream_support"
    "../../src/libs_3rdparty/libssh"
    "../../src/plugins/test_runner"
    "../../src/plugins/CoreTests"
    "../../src/plugins/perf_monitor"
    "../../src/libs_3rdparty/openssl"
)

MAP_LIST=(
    "*.cpp" "HeadC.txt"
    "*.h"   "HeadC.txt"
    "*.c"   "HeadC.txt"
    "*.cu"  "HeadC.txt"
    "*.qs"  "HeadC.txt"
    "*.js"  "HeadC.txt"
    "*.pro" "HeadPro.txt"
    "*.pri" "HeadPro.txt"
    "*.sh"  "HeadPro.txt"
)

REMOVE_FLAG=1
