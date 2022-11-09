set -e

DISTRO_LIST="arch debian ubuntu fedora"
UNAME_S=`uname -s`

if [ $UNAME_S != "Darwin" ]; then
    DISTRO=`grep -oP '(?<=^ID=).+' /etc/os-release | tr -d '"'`

    if [ $DISTRO = "raspbian" ]; then
        DISTRO="debian"
    elif ! echo $DISTRO_LIST | grep -w -q $DISTRO; then
        echo "🤖 Unknown distribution, defaulting to ubuntu"
        DISTRO="ubuntu"
    fi
fi

PLATFORM=`uname -s`-$DISTRO-`uname -m`
REPOSITORY="github.com/RainingComputers/ShnooTalk"
REPOSITORY_URL="https://$REPOSITORY"

TAGS=`git -c 'versionsort.suffix=-' ls-remote --tags --sort='v:refname' $REPOSITORY_URL`
VERSION=`echo ${TAGS##*/}`

COMPILER_SRC_TAR_DIR_NAME=`echo ShnooTalk-$VERSION`
COMPILER_SRC_TAR_NAME=`echo $COMPILER_SRC_TAR_DIR_NAME.tar.gz`
COMPILER_TAR_DIR_NAME=`echo "shtkc-$VERSION-$PLATFORM"`
COMPILER_TAR_NAME=`echo "$COMPILER_TAR_DIR_NAME.tar.gz"`
STDLIB_TAR_NAME=`echo shnootalk-stdlib-$VERSION.tar.gz`

COMPILER_SRC_RELEASE_LINK="$REPOSITORY/archive/refs/tags/$VERSION.tar.gz"
COMPILER_RELEASE_LINK="$REPOSITORY/releases/latest/download/$COMPILER_TAR_NAME"

COMPILER_BIN_DEST=`echo /usr/local/bin/shtkc-$VERSION`
COMPILER_BIN_SYMLINK_DEST=/usr/local/bin/shtkc
STDLIB_DEST=`echo /usr/local/lib/shnootalk-$VERSION/`

if [ $UNAME_S = "Darwin" ]; then 
    echo "🤖 Installing llvm@12 using brew"
    brew install llvm@12

    echo "Downloding $COMPILER_SRC_TAR_NAME"
    curl -L $COMPILER_SRC_RELEASE_LINK > $COMPILER_SRC_TAR_NAME

    echo "🤖 Extracting $COMPILER_SRC_TAR_NAME"
    tar -xf $COMPILER_SRC_TAR_NAME

    echo "🤖 Building compiler"
    make -C $COMPILER_SRC_TAR_DIR_NAME build -j 4

    echo "🤖 Installing compiler and stdlib (requires sudo)"
    sudo make -C $COMPILER_SRC_TAR_DIR_NAME install
else
    echo "🤖 Downloading $COMPILER_TAR_NAME"
    curl -OL $COMPILER_RELEASE_LINK

    echo "🤖 Extracting $COMPILER_TAR_NAME"
    tar -xf $COMPILER_TAR_NAME

    echo "🤖 Installing compiler"
    rm -f $COMPILER_BIN_DEST
    mv $COMPILER_TAR_DIR_NAME/shtkc $COMPILER_BIN_DEST
    rm -f $COMPILER_BIN_SYMLINK_DEST
    ln -s $COMPILER_BIN_DEST $COMPILER_BIN_SYMLINK_DEST

    echo "🤖 Downloading $STDLIB_TAR_NAME"
    STDLIB_RELEASE_LINK="$REPOSITORY/releases/latest/download/$STDLIB_TAR_NAME"
    curl -OL $STDLIB_RELEASE_LINK

    echo "🤖 Extracting $STDLIB_TAR_NAME"
    tar -xf $STDLIB_TAR_NAME

    echo "🤖 Installing stdlib"
    rm -rf $STDLIB_DEST
    mv stdlib/ $STDLIB_DEST
fi

echo "🤖 Removing temp files"
rm -rf $COMPILER_SRC_TAR_DIR_NAME
rm -rf $COMPILER_TAR_DIR_NAME
rm -f $COMPILER_SRC_TAR_NAME
rm -f $COMPILER_TAR_NAME
rm -f $STDLIB_TAR_NAME