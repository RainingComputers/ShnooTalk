# This script should be run from the root of the project directory
# After running this script, you will have a .tar.gz build in the bin/ directory

echo '🤖 Installing LLVM using brew'
brew install llvm@12

echo '🤖 Building compiler'
make build
BUILD_NAME=`cat build-name.txt`
cd bin/

echo '🤖 Packaging .dylib files'
cd $BUILD_NAME

cd ..

echo '🤖 Creating tar.gz'
tar cvzf $BUILD_NAME.tar.gz $BUILD_NAME

echo '✨ Done.'
cd ..
