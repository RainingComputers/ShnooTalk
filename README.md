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

These are end to end tests which can be used to test an implementation of ShnooTalk. The `tests_runner` python application is used to run the tests.

### Directories

#### compiler/tests/parser/

This is meant to test the AST generated from the parser.

#### compiler/tests/compiler/

Test the output executable produced by the compiler and the generated ShnooTalk icode (not the entire IR just the generated function IR instructions only, see `-icode-all` option).

#### compiler/tests/ir/

This is meant to test the overall structure of the generated IR (i.e the parts that were not tested in compiler/tests/compiler/ tests).

#### stdlib/stdlib/tests/

Tests for the standard library

### When to add a test?

+ When you discover a compiler bug
+ When you add a new grammar or syntax
+ If you make changes to existing syntax, you will most probably just have to fix failing tests
+ When you make changes to the IR or the IR builder
+ Avoid making changes to the pretty printer or modifying syntax of existing IR 
  instructions, but if you do, you may have to change a lot of test cases, re-generate the test cases,
  and do careful review using `git diff`

### Where to add the test?

+ If you made changes to the parser, add test to the parser/ directory
+ If you have made changes to the icode (IR instructions) or added a new IR instruction, add test to the compiler/ directory
+ If you have fixed a compiler bug, add a test to compiler/ directory
+ If you have made changes to the IR but not icode (IR instructions), add test to the ir/ directory

### How to add a test?

+ Create a .shtk files in appropriate test directory, add print statements for asserting
+ After adding the test, run `make gen FILTERS='--file <test-file-name>'`
+ Run `make test` to run all tests

### How to run the tests?

+ To run all tests, run `make test` or `make coverage`
+ To run a single file, run `make test FILTERS='--file <test-file-name>'`
+ To run only a group of tests, run `make test FILTERS='--group <group-name>'`

### How to generate the test cases?

+ Run `make gen` to generate all test cases
+ Run `make gen FILTERS='--file <test-file-name>'` to generate for a particular test file
+ Carefully review the changes using `git diff`


## Project status

### Compiler core

*Priority*

- [x] Incomplete types and type composing of a pointer to itself
- [x] null pointer check
- [x] Direct from
- [x] make constructors
- [x] := initializer
- [x] Destructuring
- [x] Generic function call
- [x] Class or struct destructors
- [x] Assignment destructuring
- [x] Better operator overloading
- [x] `in` operator
- [x] Subscript operator overloading
- [x] extern "C" to prevent name mangling functions
- [x] Decide names and defaults for primitive data types
- [x] Arrays and pointers of generic types
- [x] Proper enums
- [x] Fix bug in calling deconstructors
- [x] Generators and foreach loop
- [ ] Enumerated foreach loop
- [ ] `?` operator
- [ ] Add `clone` built in
- [ ] Reduce binary size
- [ ] Add SHNOOTALK_PATH environment variable, and use PATH by default

*Backlog*

- [x] Array return types
- [x] IR Validator
- [x] Main function return type check
- [x] Extern C functions and function imports
- [x] Node style module imports
- [x] `loop {}` syntax for looping forever
- [x] `do {} while()` loop
- [x] NaN and infinity
- [x] Multi-line strings
- [x] Assigning conditional expressions
- [x] Setup github workflows and build for other platforms
- [x] Function forward declaration
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

*Refactoring*

- [ ] Use optional
- [ ] Some duplication of code before calling `premangledFunctionCall`
- [ ] Move type checking to builder
- [ ] Remove tokens from builder
- [ ] Cleanup format strings and function call in Translator

### Documentation

- [ ] Language tour using shnootalk-playground
- [ ] Language reference in markdown

### Translators

- [ ] Debug symbols (using LLVM)
- [ ] Python wrapper generator

### Platforms

- [x] WebAssembly support
- [ ] Arduino Uno support

### Testing

- [ ] Unit tests for ir builder
- [ ] Unit tests for pretty printer
- [ ] Use multiprocessing for tests runner

### Language tools

- [ ] STPM package manager
- [ ] Primitive tests runner
- [ ] Code formatter
- [ ] Static analyzer
- [ ] Language server
- [ ] Standard library

### Self hosting and LLVM independence

- [ ] WebAssembly text translator for WASI
- [ ] Framework for writing and testing optimization passes
- [ ] C wrapper for LLVM to self host compiler
- [ ] Module IR validator
- [ ] Document IR using [pydantic](https://pydantic-docs.helpmanual.io/)
