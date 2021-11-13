# docker build -f scripts/Linux/Dockerfile .

FROM debian:latest

RUN apt -y update
RUN apt -y upgrade
RUN apt -y install tar g++ make findutils libncurses5 curl xz-utils zlib*

# Install llvm-11 for shared libraries, this llvm will not be used
RUN apt -y install llvm-11 
