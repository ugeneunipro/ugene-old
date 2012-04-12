PRODUCT_NAME="Unipro UGENE"
VERSION=`cat ../../src/ugene_version.pri | grep UGENE_VERSION | awk -F'=' '{print $2}'`
BUNDLE=./release_bundle/UGENE.app
PACKAGE_FILE=ugene-${VERSION}-mac-x86.pkg
# The folder structure under $PACKAGE_ROOT is where files will be put under installation
PACKAGE_ROOT=./build/Package_Root
APP_PATH="Applications/${PRODUCT_NAME}.app"
EXE_PATH="${APP_PATH}/Contents/MacOS"
TARGET_APP_DIR="${PACKAGE_ROOT}/${APP_PATH}"
# So under /${SYMLINKS_PATH} on users system we will place ugene, ugeneui and ugenecl symlinks leading to binaries in /${APP_PATH}
SYMLINKS_PATH="usr/bin"
SYMLINKS_DIR="${PACKAGE_ROOT}/${SYMLINKS_PATH}"
SYMLINKS_TARGET_PATH="../../${EXE_PATH}"

echo cleaning Package_Root dir
rm -rf ${PACKAGE_ROOT}

echo recreating Package_Root dir structure
mkdir -p "${TARGET_APP_DIR}"
mkdir -p "${SYMLINKS_DIR}"

if [ "$1" != "-skip-bundle" ]
then
  echo creating bundle
  ./bundle_release.sh -nodmg
fi

echo copying bundle
# NB! Ending slash in source dir is important: in BSD cp it means copy contents, not folder itself
cp -r "${BUNDLE}/" "${TARGET_APP_DIR}"

echo creating symlinks
cd "${SYMLINKS_DIR}"
ln -s "${SYMLINKS_TARGET_PATH}/ugene" ugene
ln -s "${SYMLINKS_TARGET_PATH}/ugeneui" ugeneui
ln -s "${SYMLINKS_TARGET_PATH}/ugenecl" ugenecl
cd -

# ensure script is executable
chmod +x scripts/postflight
#chmod +x ugene.sh
#cp ugene.sh "${TARGET_EXE_DIR}"

# -- Following code inspired by http://www.codeography.com/2009/09/04/automating-apple-s-packagemaker.html

# Change the permission to what they should be on the target:
#echo changing permissions
#sudo chown -R root:admin "${PACKAGE_ROOT}"
#sudo chmod -R g+w "${PACKAGE_ROOT}"


echo running packagemaker
/Developer/usr/bin/packagemaker \
--title "${PRODUCT_NAME} v.${VERSION}" \
--version $VERSION \
--filter "\.DS_Store" \
--filter "\.svn" \
--filter ".*\.swp" \
--scripts ./scripts/ \
--root-volume-only \
--domain system \
--verbose \
--no-relocate \
--install-to "/" \
--target 10.5 \
--id com.unipro.UGENE.pkg \
--root "$PACKAGE_ROOT" \
--out "$PACKAGE_FILE"

