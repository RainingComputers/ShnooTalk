set -xe

DISTRO=`grep -oP '(?<=^ID=).+' /etc/os-release | tr -d '"'`
PLATFORM=`uname -s`-$DISTRO-`uname -m`
REPOSITORY="https://github.com/RainingComputers/Shnootalk"
TAGS=`git -c 'versionsort.suffix=-' ls-remote --tags --sort='v:refname' $REPOSITORY`
VERSION=`echo ${TAGS##*/}`
BIN_DIR_NAME=`echo "shtkc-$VERSION-$PLATFORM"`
ASSET_NAME=`echo "$BIN_DIR_NAME.tar.gz"`

echo "🤖 Downloading $ASSET_NAME"
RELEASE_LINK="github.com/rainingcomputers/shnootalk/releases/latest/download/$ASSET_NAME"

curl -OL $RELEASE_LINK

echo "🤖 Extracting $ASSET_NAME"
tar -xf $ASSET_NAME

echo "🤖 Installing compiler"
mv $BIN_DIR_NAME/shtkc .

echo "🤖 Removing temp files"
rm -rf $BIN_DIR_NAME
rm -f $ASSET_NAME