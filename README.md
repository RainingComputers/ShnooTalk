![](logo.png)

# ShnooTalk 

ShnooTalk is a programming language implemented in C++ using the LLVM compiler framework. ShnooTalk works by being compiled to a custom IR format and then translating that to LLVM IR. It is possible to run ShnooTalk on any runtime that supports the clang compiler. ShnooTalk links against standard C libraries/runtime.

ShnooTalk was previously known as uHLL (micro high level language). The project is still transitioning from this name.

🛠️ Project under construction, some things may not be fully polished yet 🛠️

![](blockdiag.png)

## Installing the compiler

### Ubuntu 20.04 

```
sudo apt install llvm-12 llvm-12-dev
make build
sudo make install
```

### Mac OS

```
brew install llvm@12
make build
sudo make install
```

### VS Code syntax highlight extension

Install the extension from [here](https://marketplace.visualstudio.com/items?itemName=RainingComputers.shnootalk-vscode) 

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

## TODO

### Short term

- [x] Fix bugs in the LLVM translator backend
- [x] Array return types
- [x] Fix def
- [x] IR Validator
- [x] Change IR pretty print to JSON
- [x] IR renaming and cleanup
- [x] Main function return type check
- [x] Fix `input()` and `print()` and `println()` functions
- [x] `exit()` function
- [x] Extern C functions and function imports
- [x] Node style module imports
- [x] `loop {}` syntax for looping forever
- [x] `do {} while()` loop
- [ ] NaN and infinity
- [ ] Dynamic allocation and pointers
- [ ] Add `fopen()` function
- [ ] Multi-line strings
- [ ] Static variables
- [ ] Assigning conditional expressions
- [ ] Language reference using mkdocs
- [ ] CLI argument parser
- [ ] Refactor tests
- [ ] Refactor the test runner and use multiprocessing
- [ ] Tests for pretty printer
- [ ] ~95% test coverage
- [ ] Python wrapper generator
- [ ] WebAssembly support
- [ ] Setup github workflows and build for other platforms
- [ ] Check limits for primitive data types
- [ ] Debug symbols

### Advanced features

- [ ] Custom operators
- [ ] Constructor and destructor
- [ ] Templates
- [ ] Exceptions
- [ ] Passing functions as arguments
- [ ] Lambdas
- [ ] Unions

