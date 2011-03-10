U_NAME=ugene
U_VERSION=1.5.0
U_FULLNAME=$U_NAME-$U_VERSION

cd ../../source
./pack.sh
mv $U_FULLNAME.tar.gz ../linux/ubuntu/${U_NAME}_${U_VERSION}.orig.tar.gz
cd ../linux/ubuntu
tar -xzf ${U_NAME}_${U_VERSION}.orig.tar.gz
mkdir $U_FULLNAME/debian
cp debian/* ./$U_FULLNAME/debian
cd $U_FULLNAME/debian
chmod a+x rules
cd ..
dpkg-buildpackage
