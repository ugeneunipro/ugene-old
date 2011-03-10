#!/bin/bash


echo RUNNING FEDORA RPMBUILD SCRIPT ...
echo CHECKING OS...
if ! uname -a | grep -i "fedora" >/dev/null 2>&1;
then
    echo The OS is not Fedora.
    echo EXITING FEDORA RPMBUILD SCRIPT ...
    exit 0
fi
echo The OS is Fedora: CHECK.

# ... CONFIGURING BUILD ...
 echo CONFIGURING BUILD ...
 echo "##teamcity[blockOpened name='Conf build']"
UINST_DIR=`pwd`
cd ../../source
INST_SRC_DIR=`pwd`
U_VERSION=`grep '^U_VERSION' config.sh | cut -d '=' -f2`
U_NAME=`grep '^U_NAME' config.sh | cut -d '=' -f2 `
U_FULLNAME=$U_NAME-$U_VERSION
cd ../../src
SRC_DIR=`pwd`
cd ~
HOME_DIR=`pwd`
DATE=_new
RPM_MACROS=$HOME_DIR"/.rpmmacros"
RPM_DIR=$HOME_DIR"/rpm"$DATE
RPM_PATH="%_topdir $RPM_DIR"
cd $INST_SRC_DIR
TARBALL=$INST_SRC_DIR/$U_FULLNAME.tar.gz
 echo "##teamcity[blockOpened name='Conf build']"


# ... ENABLING EXCLUDE LIST ...
 echo "##teamcity[blockOpened name='Exclude list']"
UGLOB_PRI=ugene_globals.pri
cp -f $SRC_DIR/$UGLOB_PRI $UINST_DIR/$UGLOB_PRI
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


# ... PREPARING RPM-BUILD DIRECTORIES ...
 echo PREPARING RPM-BUILD DIRECTORIES ...
 echo "##teamcity[blockOpened name='Preparing DEB-build dirs']"
if [ -e $RPM_MACROS ]
then
    RPM_MACROS_TMP=$RPM_MACROS$DATE
    mv $RPM_MACROS $RPM_MACROS_TMP
fi
echo $RPM_PATH >$RPM_MACROS
mkdir $RPM_DIR

if [ ! -d $RPM_DIR/BUILD ];     then mkdir $RPM_DIR/BUILD
fi
if [ ! -d $RPM_DIR/BUILDROOT ]; then mkdir $RPM_DIR/BUILDROOT
fi
if [ ! -d $RPM_DIR/RPMS ];      then mkdir $RPM_DIR/RPMS
fi
if [ ! -d $RPM_DIR/SOURCES ];   then mkdir $RPM_DIR/SOURCES
fi
if [ ! -d $RPM_DIR/SPECS ];     then mkdir $RPM_DIR/SPECS
fi
if [ ! -d $RPM_DIR/SRPMS ];     then mkdir $RPM_DIR/SRPMS
fi
 echo "##teamcity[blockClosed name='Preparing DEB-build dirs']"


# ... RPM-BUILD ...
 echo RPM_BUILD ...
 echo "##teamcity[blockOpened name='RPM-build']"
cp $TARBALL $RPM_DIR/SOURCES
if [ ! $WAS_TARBALL ]; then rm $TARBALL
fi
cp $UINST_DIR/ugene.spec $RPM_DIR/SPECS
cd $RPM_DIR/SPECS
rpmbuild -bb ugene.spec
cp -rf $RPM_DIR/RPMS/* $UINST_DIR
 echo "##teamcity[blockClosed name='RPM-build']"
fi


if [ "$UGENE_DISTR_SKIP_INSTALL" -ne "1" ]; then #UGENE_DISTR_SKIP_BUILD
# ... (RE)INSTALLING BUILT RPM ...
echo "(RE)INSTALLING BUILT RPM ..."
 echo "##teamcity[blockOpened name='Installing RPMs']"
U_RELEASENUM=`grep 'Release' $UINST_DIR/ugene.spec | cut -d ':' -f2 | cut -d '%' -f1 | sed 's/ //'`
PKG_NAME=$U_FULLNAME-$U_RELEASENUM.fc11.i586
PKG_FILE=$UINST_DIR/i586/$PKG_NAME.rpm
sudo yum -y erase $U_NAME
sudo rpm -i $PKG_FILE
 echo "##teamcity[blockClosed name='Installing RPMs']"
fi


# ... CLEANING UP ...
 echo CLEANING UP ...
 echo "##teamcity[blockOpened name='Clean after RPM-build']"
mv -f $UINST_DIR/$UGLOB_PRI $SRC_DIR/$UGLOB_PRI
if [ -d $RPM_DIR ]
then
    rm -rf $HOME_DIR"/rpm_old"
    cp -rf $RPM_DIR $HOME_DIR"/rpm_old"
    rm -rf $RPM_DIR
fi
if [ $RPM_MACROS_TMP ];
then 
    rm $RPM_MACROS
    mv $RPM_MACROS_TMP $RPM_MACROS
fi
 echo "##teamcity[blockClosed name='Clean after RPM-build']"


if [ "$UGENE_DISTR_SKIP_TESTRUNNER" -ne "1" ]; then #UGENE_DISTR_SKIP_TESTRUNNER
# ... COMPILING AND INSTALLING TeST RUNNER ...
 echo COMPILING AND INSTALLING TeST RUNNER ...
 echo "##teamcity[blockOpened name='Installing plugin_test_runner']"
cd $SRC_DIR/core
qmake-qt4 -r
make
cd $SRC_DIR/plugins/test_runner
qmake-qt4 -r
make
sudo cp -f $SRC_DIR/_release/plugins/libtest_runner.so.1.0.0 /usr/lib64/ugene/plugins
sudo ln -s /usr/lib/ugene/plugins/libtest_runner.so.1.0.0 /usr/lib64/ugene/plugins/libtest_runner.so.1.0
sudo ln -s /usr/lib/ugene/plugins/libtest_runner.so.1.0.0 /usr/lib64/ugene/plugins/libtest_runner.so.1
sudo ln -s /usr/lib/ugene/plugins/libtest_runner.so.1.0.0 /usr/lib64/ugene/plugins/libtest_runner.so
 echo "##teamcity[blockClosed name='Installing plugin_test_runner']"
fi


# ... RUNNING TESTS ...
 echo RUNNING TESTS ...
 echo "##teamcity[blockOpened name='Running tests']"
cd $SRC_DIR/..
ROOT_DIR=`pwd`
export TEST_PATH="$ROOT_DIR/test/"
if [ -z "$TEST_SUITE" ]; then
  export TEST_SUITE=nightlyExcludeTests.list
fi
rm -f $HOME_DIR/.config/Unipro/UGENE.ini
if [ -e $ROOT_DIR/test/test_report.html ];then
  rm -f $ROOT_DIR/test/test_report.html
fi
echo "##teamcity[blockOpened name='Environment']"
  env
echo "##teamcity[blockClosed name='Environment']"
if
ugene ./test/gui/component/plugins/TestRunner/run_suites.js
then
  echo "##teamcity[buildStatus status='SUCCESS' text='{build.status.text}']"
else
  echo "##teamcity[buildStatus status='FAILURE' text='{build.status.text}. Testing Failed.']"
fi
 echo "##teamcity[blockClosed name='Running tests']"


# ... EXITING FEDORA RPMBUILD SCRIPT ...
 echo EXITING FEDORA RPMBUILD SCRIPT ...
exit 0