# This script should be run from the root of the project directory
# After running this script, you will have a .tar.gz build in the bin/ directory

set -e

echo '🤖 Installing llvm@12 using brew'
brew install llvm@12

echo '🤖 Building compiler'
make build -j 2
BUILD_NAME=`cat build-name.txt`

echo '🤖 Rewiring and moving .dylib files'
python3 scripts/MacOS-x86_64/dylib_rewire.py bin/$BUILD_NAME/shtkc

echo '🤖 Checking executable'
bin/$BUILD_NAME/shtkc -version

echo '🤖 Creating tar.gz'
cd bin/
tar cvzf $BUILD_NAME.tar.gz $BUILD_NAME
cd ..

echo '✨ Done.'
