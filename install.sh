set -e

DISTRO_LIST="arch debian ubuntu fedora"
DISTRO=`grep -oP '(?<=^ID=).+' /etc/os-release | tr -d '"'`

if [ $DISTRO = "raspbian" ]; then
    DISTRO="debian"
elif ! echo $DISTRO_LIST | grep -w -q $DISTRO; then
    echo "🤖 Unknown distribution, defaulting to ubuntu"
    DISTRO="ubuntu"
fi
PLATFORM=`uname -s`-$DISTRO-`uname -m`
REPOSITORY="https://github.com/RainingComputers/Shnootalk"
TAGS=`git -c 'versionsort.suffix=-' ls-remote --tags --sort='v:refname' $REPOSITORY`
VERSION=`echo ${TAGS##*/}`
BIN_DIR_NAME=`echo "shtkc-$VERSION-$PLATFORM"`
COMPILER_TAR_NAME=`echo "$BIN_DIR_NAME.tar.gz"`
STDLIB_TAR_NANE=`echo shnootalk-stdlib-$VERSION.tar.gz`

echo "🤖 Downloading $COMPILER_TAR_NAME"
COMPILER_RELEASE_LINK="github.com/rainingcomputers/shnootalk/releases/latest/download/$COMPILER_TAR_NAME"
curl -OL $COMPILER_RELEASE_LINK

echo "🤖 Extracting $COMPILER_TAR_NAME"
tar -xf $COMPILER_TAR_NAME

echo "🤖 Installing compiler"
mv $BIN_DIR_NAME/shtkc /usr/local/bin

echo "🤖 Downloading $STDLIB_TAR_NAME"
STDLIB_RELEASE_LINK="github.com/rainingcomputers/shnootalk/releases/latest/download/$STDLIB_TAR_NAME"
curl -OL $STDLIB_RELEASE_LINK

echo "🤖 Extracting $STDLIB_TAR_NAME"
tar -xf $STDLIB_TAR_NAME

echo "🤖 Installing compiler"
mv stdlib/ /usr/local/lib/shnootalk-$VERSION/

echo "🤖 Removing temp files"
rm -rf $BIN_DIR_NAME
rm -f $COMPILER_TAR_NAME
