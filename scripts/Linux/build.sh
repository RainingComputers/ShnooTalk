# This script should be run from the root of the project directory
# After running this script, you will have a .tar.gz build in the bin/ directory

set -e

TAR_NAME=`basename $1`

if [ ! -f "$TAR_NAME" ]
then
    echo '🤖 Downloading LLVM release'
    curl -OL https://github.com/llvm/llvm-project/releases/download/$1

    echo '🤖 Extracting release'
    mkdir -p llvm
    tar -xf $TAR_NAME -C llvm --strip-components 1
fi

#echo '🤖 Building compiler'
#make build LLVM_PATH=llvm -j 8
#BUILD_NAME=`cat build-name.txt`
#
#echo '🤖 Checking executable'
#bin/$BUILD_NAME/shtkc -version
#
#echo '🤖 Creating tar.gz'
#cd bin/
#tar cvzf $BUILD_NAME.tar.gz $BUILD_NAME
#cd ..
#
#echo '✨ Done.'
