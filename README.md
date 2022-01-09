![](logo.png)

# ShnooTalk 

ShnooTalk is a programming language implemented in C++ using the LLVM compiler framework. ShnooTalk works by being compiled to a custom IR format and then translating that to LLVM IR. It is possible to run ShnooTalk on any runtime that supports the clang compiler. ShnooTalk links against standard C libraries/runtime.

See the tests directory for examples, try out the language in your browser on [ShnooTalk Playground](http://rainingcomputers.gitlab.io/shnootalk-playground/).

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

## TODO

### Language core

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
- [ ] Cleanup format strings and function call in Translator
- [ ] Static variables
- [ ] Language tour using shnootalk-playground
- [ ] Language reference in markdown
- [x] Setup github workflows and build for other platforms
- [ ] Error recovery
- [ ] Incomplete types and type composing of a pointer to itself
- [ ] Destructuring
- [ ] := initializer
- [ ] Function forward declaration
- [ ] Direct from
- [ ] Unsafe keyword
- [ ] null pointer check
- [ ] Generic function call


### Advanced features

- [x] Custom operators
- [ ] Generics
- [ ] Standard library
- [ ] Passing functions as arguments
- [ ] Constructor and destructor
- [ ] Exceptions
- [ ] Unions

### Translators

- [ ] Debug symbols
- [ ] Python wrapper generator
- [ ] WebAssembly support

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
- [ ] Code formatter
- [ ] Static analyser
- [ ] Language server
