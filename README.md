![](logo.png)

# ShnooTalk 
ShnooTalk is a programming language implemented in C++ using the LLVM compiler framework. ShnooTalk works by being compiled to a custom IR format and then translating that to LLVM IR. It is possible to run ShnooTalk on any runtime that supports the clang compiler. ShnooTalk links against standard C libraries/runtime.

ShnooTalk was previously known as uHLL (micro high level language). The project is still transitioning from this name.

🛠️ Project under construction, some things may not be fully polished yet 🛠️

## Installing the compiler

### Ubuntu 20.04 
```
sudo apt install llvm-11 llvm-11-dev
make build
sudo make install
```

## Getting started

**app.uhll**
```
fn main() -> int
{
    println("Hello world")
    return 0
}
```

**Compile and run**
```
uhll app.uhll -c
clang app.o -o app
./app
```
