# This script should be run from the root of the project directory
# After running this script, you will have a .tar.gz build in the bin/ directory

echo '🤖 Downloading LLVM release'
curl -OL https://github.com/llvm/llvm-project/releases/download/$1

echo '🤖 Extracting release'
mkdir -p llvm
tar -xf `basename $1` -C llvm --strip-components 1

echo '🤖 Building compiler'
make build
BUILD_NAME=`cat build-name.txt`
cd bin/

echo '🤖 Creating tar.gz'
tar cvzf $BUILD_NAME.tar.gz $BUILD_NAME

echo '✨ Done.'
cd ..
