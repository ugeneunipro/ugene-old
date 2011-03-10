#!/bin/bash


echo RUNNING UBUNTU DPGK BUILD SCRIPT ...
echo CHECKING OS...
if ! uname -a | grep -i "ubuntu" >/dev/null 2>&1;
then
    echo The OS is not Ubuntu.
    echo EXITING UBUNTU DPKG BUILD SCRIPT ...
    exit 0
fi
echo The OS is Ubuntu: CHECK.

# ... CONFIGURING BUILD ...
 echo CONFIGURING BUILD ...
 echo "##teamcity[blockOpened name='Conf build']"
if [ ! -d distr ]; then mkdir distr
fi
cd distr
echo "build.sh: cd "`pwd`
UINST_DIR=`pwd`
cd ../../../source
echo "build.sh: cd "`pwd`
INST_SRC_DIR=`pwd`
U_VERSION=`grep '^U_VERSION' config.sh | cut -d '=' -f2`
U_NAME=`grep '^U_NAME' config.sh | cut -d '=' -f2 `
U_FULLNAME=$U_NAME-$U_VERSION
cd ../../src
echo "build.sh: cd "`pwd`
SRC_DIR=`pwd`
cd ~
echo "build.sh: cd "`pwd`
HOME_DIR=`pwd`
DATE=_new
cd $INST_SRC_DIR
echo "build.sh: cd "`pwd`
TARBALL=$INST_SRC_DIR/$U_FULLNAME.tar.gz
 echo "##teamcity[blockClosed name='Conf build']"


# ... ENABLING EXCLUDE LIST ...
 echo ENABLING EXCLUDE LIST ...
 echo "##teamcity[blockOpened name='Exclude list']"
UGLOB_PRI=ugene_globals.pri
echo "build.sh: mv $SRC_DIR/$UGLOB_PRI $UINST_DIR/$UGLOB_PRI"
mv $SRC_DIR/$UGLOB_PRI $UINST_DIR/$UGLOB_PRI
if [ "$UGENE_EXCLUDE_LIST_ENABLED" -eq "1" ]
then
  echo "build.sh: cat $UINST_DIR/$UGLOB_PRI | sed 's/UGENE_EXCLUDE_LIST_ENABLED *= *0/UGENE_EXCLUDE_LIST_ENABLED = 1/' >$SRC_DIR/$UGLOB_PRI"
  cat $UINST_DIR/$UGLOB_PRI | sed 's/UGENE_EXCLUDE_LIST_ENABLED *= *0/UGENE_EXCLUDE_LIST_ENABLED = 1/' >$SRC_DIR/$UGLOB_PRI
elif [ "$UGENE_EXCLUDE_LIST_ENABLED" -eq "0" ]
then
  echo "build.sh: cat $UINST_DIR/$UGLOB_PRI | sed 's/UGENE_EXCLUDE_LIST_ENABLED *= *1/UGENE_EXCLUDE_LIST_ENABLED = 0/' >$SRC_DIR/$UGLOB_PRI"
  cat $UINST_DIR/$UGLOB_PRI | sed 's/UGENE_EXCLUDE_LIST_ENABLED *= *1/UGENE_EXCLUDE_LIST_ENABLED = 0/' >$SRC_DIR/$UGLOB_PRI
fi
 echo "##teamcity[blockClosed name='Exclude list']"

if [ "$UGENE_DISTR_SKIP_BUILD" -ne "1" ]; then #UGENE_DISTR_SKIP_BUILD
# ... CREATING TARBALL ...
 echo CREATING TARBALL ...
 echo "##teamcity[blockOpened name='Creating tarball']"
echo TARBALL=$TARBALL
if [ -e $TARBALL ];
then
    rm $TARBALL
    WAS_TARBALL=1
fi
. ./pack.sh
 echo "##teamcity[blockClosed name='Creating tarball']"


# ... PREPARING DPKG-BUILD DIRECTORIES ...
 echo PREPARING DPKG-BUILD DIRECTORIES ...
 echo "##teamcity[blockOpened name='Preparing DEB-build dirs']"
cd $UINST_DIR
echo "build.sh: cd "`pwd`
if [ ! -d debian ]; then mkdir debian
fi
echo "build.sh: cp $TARBALL $UINST_DIR"
cp $TARBALL $UINST_DIR
 echo "##teamcity[blockClosed name='Preparing DEB-build dirs']"

# ... DPKG-BUILD ...
 echo DPKG-BUILD ...
 echo "##teamcity[blockOpened name='DEB-build']"
echo "build.sh: tar -xzf $U_FULLNAME.tar.gz"
tar -xzf $U_FULLNAME.tar.gz
cd $U_FULLNAME
echo "build.sh: cd "`pwd`
echo "build.sh: cp installer/linux/ubuntu/debian/* ../debian/"
cp installer/linux/ubuntu/debian/* ../debian/
rm -rf bkm
rm -rf installer/linux
rm -rf installer/windows
rm -rf installer/macosx
rm -rf src/libs_3rdparty/zlib
rm -rf src/libs_3rdparty/openssl
cd ../
echo "build.sh: cd "`pwd`
echo "build.sh: mkdir $U_FULLNAME/debian"
if [ ! -d $U_FULLNAME/debian ]; then mkdir $U_FULLNAME/debian
fi
echo "bldscipt: tar -czf $U_FULLNAME+repack.orig.tar.gz $U_FULLNAME"
tar -czf $U_FULLNAME+repack.orig.tar.gz $U_FULLNAME
echo "build.sh: cp debian/* $U_FULLNAME/debian/"
cp debian/* $U_FULLNAME/debian/
cat debian/rules | sed 's/qmake-qt4 -r/qmake-qt4 -r CONFIG+=x64/' >$U_FULLNAME/debian/rules
sudo chmod u+x ./$U_FULLNAME/debian/rules
cd $U_FULLNAME
echo "build.sh: cd "`pwd`
echo "build.sh: dpkg-buildpackage -kD71359B0"
dpkg-buildpackage -j4 #-kD71359B0
#rm -rf debian
#key id: D71359B0
#ignore deps: -d
#source:
#dpkg-buildpackage -S -kD71359B0
#all:
#dpkg-buildpackage -kD71359B0
 echo "##teamcity[blockClosed name='DEB-build']"
fi #UGENE_DISTR_SKIP_BUILD


if [ "$UGENE_DISTR_SKIP_INSTALL" -ne "1" ]; then #UGENE_DISTR_SKIP_BUILD
# ... (RE)INSTALLING BUILT DEBs ...
echo "(RE)INSTALLING BUILT DEBs ..."
 echo "##teamcity[blockOpened name='Installing DEBs']"
cd $UINST_DIR
echo "build.sh: cd "`pwd`
echo "build.sh: sudo apt-get -q=2 remove ugene"
sudo apt-get -q=2 remove ugene
echo "build.sh: sudo apt-get -q=2 remove ugene-data"
sudo apt-get -q=2 remove ugene-data
echo "build.sh: dpkg -i $U_NAME-data_$U_VERSION+repack-0ubuntu1_all.deb"
sudo dpkg -i $U_NAME-data_$U_VERSION+repack-0ubuntu1_all.deb
echo "build.sh: dpkg -i ${U_NAME}_${U_VERSION}+repack-0ubuntu1_amd64.deb"
sudo dpkg -i ${U_NAME}_${U_VERSION}+repack-0ubuntu1_amd64.deb
 echo "##teamcity[blockClosed name='Installing DEBs']"
fi #UGENE_DISTR_SKIP_INSTALL


# ... CLEANING UP ...
 echo CLEANING UP ...
 echo "##teamcity[blockOpened name='Clean after DEB-build']"
echo "build.sh: mv -f $UINST_DIR/$UGLOB_PRI $SRC_DIR/$UGLOB_PRI"
mv -f $UINST_DIR/$UGLOB_PRI $SRC_DIR/$UGLOB_PRI
 echo "##teamcity[blockClosed name='Clean after DEB-build']"


if [ "$UGENE_DISTR_SKIP_TESTRUNNER" -ne "1" ]; then #UGENE_DISTR_SKIP_TESTRUNNER
# ... COMPILING AND INSTALLING TeST RUNNER ...
 echo COMPILING AND INSTALLING TeST RUNNER ...
 echo "##teamcity[blockOpened name='Installing plugin_test_runner']"
cd $SRC_DIR/core
echo "build.sh: cd "`pwd`
echo "build.sh: qmake -r CONFIG+=x64"
qmake -r CONFIG+=x64
make
cd $SRC_DIR/plugins/test_runner
echo "build.sh: cd "`pwd`
echo "build.sh: qmake -r CONFIG+=x64"
qmake -r CONFIG+=x64
echo "build.sh: make"
make
echo "build.sh: sudo cp $SRC_DIR/_release/plugins/libtest_runner.so.1.0.0 /usr/lib/ugene/plugins"
if [ -e /usr/lib/ugene/plugins/libtest_runner.so.1.0.0 ]
then
  sudo rm -f /usr/lib/ugene/plugins/libtest_runner.so.1.0.0
  sudo rm -f /usr/lib/ugene/plugins/libtest_runner.so.1.0
  sudo rm -f /usr/lib/ugene/plugins/libtest_runner.so.1
  sudo rm -f /usr/lib/ugene/plugins/libtest_runner.so
fi
sudo cp /home/plummet/ugene/src/_release/plugins/libtest_runner.so.1.0.0 /usr/lib/ugene/plugins
sudo ln -s /usr/lib/ugene/plugins/libtest_runner.so.1.0.0 /usr/lib/ugene/plugins/libtest_runner.so.1.0
sudo ln -s /usr/lib/ugene/plugins/libtest_runner.so.1.0.0 /usr/lib/ugene/plugins/libtest_runner.so.1
sudo ln -s /usr/lib/ugene/plugins/libtest_runner.so.1.0.0 /usr/lib/ugene/plugins/libtest_runner.so
 echo "##teamcity[blockClosed name='Installing plugin_test_runner']"
fi #UGENE_DISTR_SKIP_TESTRUNNER


# ... RUNNING TESTS ...
 echo RUNNING TESTS ...
 echo "##teamcity[blockOpened name='Running tests']"

cd $SRC_DIR/..
echo "build.sh: cd "`pwd`
ROOT_DIR=`pwd`
export TEST_PATH="$ROOT_DIR/test/"
echo "build.sh: given TEST_SUITE=$TEST_SUITE"
if [ -z "$TEST_SUITE" ]; then
  export TEST_SUITE=nightlyExcludeTests.list
fi
echo "build.sh: resulting TEST_SUITE=$TEST_SUITE"
echo "build.sh: rm -f $HOME_DIR/.config/Unipro/UGENE.ini"
rm -f $HOME_DIR/.config/Unipro/UGENE.ini
if [ -e $ROOT_DIR/test/test_report.html ];then
  echo "build.sh: rm -f $ROOT_DIR/test/test_report.html"
  rm -f $ROOT_DIR/test/test_report.html
fi
  #env
echo "build.sh: ugene ./test/gui/component/plugins/TestRunner/run_suites.js"

if
    ugene ./test/gui/component/plugins/TestRunner/run_suites.js
then
    echo "##teamcity[buildStatus status='SUCCESS' text='{build.status.text}']"    
else
    echo "##teamcity[buildStatus status='FAILURE' text='{build.status.text}. Testing failed']"
fi

echo "##teamcity[blockClosed name='Running tests']"

if [ "$UGENE_DISTR_SKIP_INSTALL" -ne "1" ]; then #UGENE_DISTR_SKIP_BUILD
# ... UNINSTALLING INSTALLED DEBs ...
echo "UNINSTALLING INSTALLED DEBs ..."
 echo "##teamcity[blockOpened name='Uninstalling DEBs']"
echo "build.sh: sudo apt-get -q=2 remove ugene"
sudo apt-get -q=2 remove ugene
echo "build.sh: sudo apt-get -q=2 remove ugene-data"
sudo apt-get -q=2 remove ugene-data
 echo "##teamcity[blockClosed name='Uninstalling DEBs']"
fi

# ... EXITING UBUNTU DEBBUILD SCRIPT ...
 echo EXITING UBUNTU DEBBUILD SCRIPT ...
exit 0
