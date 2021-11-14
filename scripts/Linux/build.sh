# This script should be run from the root of the project directory
# After running this script, you will have a .tar.gz build in the bin/ directory

set -e

echo '🤖 Downloading LLVM release'
curl -OL $1

echo '🤖 Extracting release'
mkdir -p llvm
tar -xf `basename $1` -C llvm --strip-components 1

echo '🤖 Building compiler'
make build LLVM_PATH=llvm -j 8
BUILD_NAME=`cat build-name.txt`

echo '🤖 Checking executable'
bin/$BUILD_NAME/shtkc -version

echo '🤖 Creating tar.gz'
cd bin/
tar cvzf $BUILD_NAME.tar.gz $BUILD_NAME
cd ..

echo '✨ Done.'
