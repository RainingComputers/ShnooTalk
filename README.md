![](logo.png)

# ShnooTalk 

ShnooTalk is a programming language implemented in C++ using the LLVM compiler framework. ShnooTalk works by being compiled to a custom IR format and then translating that to LLVM IR. It is possible to run ShnooTalk on any runtime that supports the clang compiler. ShnooTalk links against standard C libraries/runtime.

See the tests directory for examples, try out the language in your browser on [ShnooTalk Playground](https://rainingcomputers.github.io/shnootalk-playground/).

🛠️ Project under construction, some things may not be fully polished yet 🛠️

![](blockdiag.png)

## Install

### Linux (Ubuntu / Arch / Debian / Fedora / RaspberryPi)
```
curl -sf -L https://raw.githubusercontent.com/RainingComputers/ShnooTalk/main/install.sh | sudo sh
```

### MacOS
```
brew tap rainingcomputers/shnootalk
brew install shtkc
```

### Linux x86_64 AppImage
Download app image from [releases](https://github.com/RainingComputers/ShnooTalk/releases) page

### Installing from source (Ubuntu)
Download the source from latest [release](https://github.com/RainingComputers/ShnooTalk/releases)
```
sudo apt install llvm-12 llvm-12-dev
make build
sudo make install
```

### Installing from source (MacOS)
Download the source from latest [release](https://github.com/RainingComputers/ShnooTalk/releases)
```
brew install llvm@12
make build
sudo make install
```

### VS Code syntax highlight extension

Install the extension from [here](https://marketplace.visualstudio.com/items?itemName=RainingComputers.shnootalk-vscode) 


## Getting started

**app.shtk**
```
fn main() -> int
{
    println("Hello world")
    return 0
}
```

**Compile and run**
```
shtkc app.shtk -c
clang app.o -o app
./app
```
(You can also use gcc instead of clang)

## Running compiler tests

```
make test
```

## Project status

### Compiler core

*Priority*
- [x] Incomplete types and type composing of a pointer to itself
- [x] null pointer check
- [x] Direct from
- [x] make constructors
- [x] := initializer
- [x] Destructuring
- [x] Clean up packaging process
- [x] Improve error messages and module trace list
- [x] Change compiler tests to use `-icode-all`
- [x] Generic function call
- [x] Fix sizeof
- [x] Class or struct destructors
- [x] Pretty name for generics
- [x] Assignment destructuring
- [x] Better operator overloading
- [x] `in` operator
- [x] Subscript operator overloading
- [ ] Reduce binary size
- [ ] Fix bug in calling deconstructors
- [ ] Generators and foreach loop
- [x] extern "C" to prevent name mangling functions
- [x] Decide names and defaults for primitive data types
- [x] Arrays and pointers of generic types
- [x] Proper enums

*Backlog*
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
- [x] NaN and infinity
- [x] 01234 bug
- [x] Check limits for primitive data types
- [x] Dynamic allocation and pointers
- [x] Add `fopen()` function
- [x] Multi-line strings
- [x] Assigning conditional expressions
- [x] Setup github workflows and build for other platforms
- [x] Function forward declaration
- [ ] Cleanup format strings and function call in Translator
- [ ] Error recovery in lexer and parser
- [ ] Static variables
- [ ] Struct alignment and padding
- [ ] Initializing global variables
- [ ] typedef
- [ ] Better error message for using generic as non generic
- [ ] Autofill type parameter for generic function call
- [ ] Using literal expression as subscript for declaration
- [ ] Struct construction using `Foobar{}` syntax
- [ ] `panic()` function
- [ ] Upgrade to using opaque pointers (LLVM IR)
- [ ] Add linker

*Advanced Features*

- [x] Custom operators
- [x] Generics
- [ ] Closures and lambdas
- [ ] Exceptions
- [ ] Unions
- [ ] Interfaces
- [ ] Extends
- [ ] Compile time reflection and macros
- [ ] `constexpr` or `comptime`
- [ ] Memory safety (rust's borrow checker?)
- [ ] Testing framework
- [ ] Concurrency

### Documentation

- [ ] Language tour using shnootalk-playground
- [ ] Language reference in markdown

### Translators

- [ ] Debug symbols (using LLVM)
- [ ] Python wrapper generator
- [ ] Switch to using opaque pointers and `getelementptr` for LLVM

### Platforms

- [x] WebAssembly support
- [ ] Arduino Uno support

### Testing
- [x] Refactor tests
- [x] Refactor the test runner 
- [x] ~95% test coverage
- [x] Tests for ast and ir
- [ ] Unit tests for ir builder
- [ ] Unit tests for pretty printer
- [ ] Use multiprocessing for tests runner

### Language tools

- [ ] STPM package manager
- [ ] Primitive tests runner
- [ ] Code formatter
- [ ] Static analyser
- [ ] Language server
- [ ] Standard library

### Self hosting and LLVM independence

- [ ] WebAssembly text translator for WASI
- [ ] Framework for writing and testing optimization passes
- [ ] C wrapper for LLVM to self host compiler
- [ ] Module IR validator
- [ ] Document IR using [pydantic](https://pydantic-docs.helpmanual.io/)
