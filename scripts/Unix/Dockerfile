# Should be build from root of the project directory
# docker build -f scripts/Unix/ .

FROM fedora:latest

ARG LLVM_RELEASE=llvmorg-12.0.1/clang+llvm-12.0.1-x86_64-linux-gnu-ubuntu-16.04.tar.xz

COPY . /home/
WORKDIR /home

RUN scripts/Unix/build.sh ${LLVM_RELEASE}
