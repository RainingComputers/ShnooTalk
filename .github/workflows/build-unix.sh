echo '-=[Downloading LLVM release]=-'
curl -OL https://github.com/llvm/llvm-project/releases/download/$1

echo '-=[Extracting release]=-'
mkdir -p llvm
tar -xf `basename $1` -C llvm --strip-components 1

echo '-=[Building compiler]=-.'
make build LLVM_PATH=llvm CXX=$2

echo '-=[Creating tar.gz]=-'
cd bin/ && tar cvzf `ls`.tar.gz `ls` && cd ..

echo 'Done.'
