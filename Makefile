.PHONY : help clean all build install install-gedit uninstall format test coverage
help :
	@echo "clean"
	@echo "      Remove auto-generated files."
	@echo "build"
	@echo "      Build release executable."
	@echo "build DEBUG=1"
	@echo "      Build executable for debugging."
	@echo "build GPROF=1"
	@echo "      Build executable for gprof (code profiling)."
	@echo "build GCOV=1"
	@echo "      Build executable for gcov (code coverage)."
	@echo "install"
	@echo "      Installs the executable to /usr/local/bin directory. Execute 'make build' first."
	@echo "install-gedit"
	@echo "      Installs syntax highlighting rule file for gedit"
	@echo "uninstall"
	@echo "      Uninstalls the executable from /usr/local/bin directory."
	@echo "format"
	@echo "      Run clang-format."
	@echo "test"
	@echo "      Run tests, run make build DEBUG=1 first"
	@echo "coverage"
	@echo "      Run test and prepare code coverage report, run make build GCOV=1"
	@echo ""

# Name of the executable
EXEC_NAME = uhll

# C++ compiler
CXX ?= clang++

# Get platform
ifeq ($(OS), Windows_NT)
	PLATFORM = $(OS)_$(PROCESSOR_ARCHITECTURE)
	EXEC_NAME := $(EXEC_NAME).exe
else ifeq ($(CXX), x86_64-w64-mingw32-g++)
	PLATFORM = Windows_NT_AMD64
	EXEC_NAME := $(EXEC_NAME).exe
else
	PLATFORM = $(shell uname -s)_$(shell uname -p)
endif

# Set C++ compiler flags, build type and build path
DEBUG ?= 0
GPROF ?= 0
GCOV ?= 0
ifeq ($(DEBUG), 1)
    CXXFLAGS = -g
	BUILD_TYPE = debug
else ifeq ($(GPROF), 1)
    CXXFLAGS = -pg -g
	BUILD_TYPE = gprof
else ifeq ($(GCOV), 1)
    LDFLAGS = -lgcov --coverage
    CXXFLAGS = -fprofile-arcs -ftest-coverage -g
	BUILD_TYPE = gcov
else
    CXXFLAGS = -O3
	BUILD_TYPE = release_$(PLATFORM)
endif

# llvm-config bin path, used to set llvm linker flags
LLVM_CONFIG_BIN = llvm-config-12

ifeq ($(shell uname -s), Darwin)
	LLVM_CONFIG_BIN = /usr/local/opt/llvm@12/bin/llvm-config
endif

# Set compiler and linker flags for llvm
CXXFLAGS := $(CXXFLAGS) -I`$(LLVM_CONFIG_BIN) --includedir` --std=c++17  -Wall
LDFLAGS := $(LDFLAGS) `$(LLVM_CONFIG_BIN) --ldflags --system-libs --libs all`

# Find all .cpp files in src/git
SOURCES = $(shell find src/ -name '*.cpp')
# Set object file names, all object files are in obj/
OBJECTS = $(SOURCES:src/%.cpp=obj/$(BUILD_TYPE)/%.o)

# Remove object files and executable
clean:
	rm -f -r bin/
	rm -f -r obj/
	rm -f -r tests/testinfo/
	rm -f tests/*.llc
	rm -f tests/*.llc.s
	rm -f tests/*.o
	rm -f tests/test

# For compiling .cpp files in src/ to .o object files in obj/
obj/$(BUILD_TYPE)/%.o: src/%.cpp src/*/*.hpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# For creting directories required for linking and building executable
dirs:
	mkdir -p obj/$(BUILD_TYPE)/
	mkdir -p bin/$(BUILD_TYPE)/

# Linking all object files to executable 
bin/$(BUILD_TYPE)/$(EXEC_NAME): $(OBJECTS)
	$(CXX) -o bin/$(BUILD_TYPE)/$(EXEC_NAME) $(OBJECTS) $(LDFLAGS)

all: dirs bin/$(BUILD_TYPE)/$(EXEC_NAME) 

# Build executable
build: all

install:
	cp bin/$(BUILD_TYPE)/$(EXEC_NAME) /usr/local/bin

install-gedit:
	cp uhll.lang /usr/share/gtksourceview-4/language-specs/uhll.lang

uninstall:
	rm /usr/local/bin/$(EXEC_NAME)

format:
	clang-format -i src/*/*/*.cpp
	clang-format -i src/*/*/*.cpp
	clang-format -i src/*/*.cpp
	clang-format -i src/*/*.hpp
	clang-format -i src/*.cpp

test:
	python3 test.py

coverage:
	python3 test.py --coverage

profile:
	python3 test.py --profile

tidy:
	clang-tidy src/*.cpp src/*/*.cpp  -- $(CXXFLAGS) -Wextra
