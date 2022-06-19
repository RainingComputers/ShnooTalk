.PHONY : help build install install-gedit uninstall format format-dry-run test coverage tidy clean appdir
help :
	@echo "clean"
	@echo "      Remove auto-generated files"
	@echo "build"
	@echo "      Build release executable"
	@echo "build DEBUG=1"
	@echo "      Build executable for debugging"
	@echo "build GPROF=1"
	@echo "      Build executable for gprof (code profiling)"
	@echo "build GCOV=1"
	@echo "      Build executable for gcov (code coverage)"
	@echo "install"
	@echo "      Installs the executable to /usr/local/bin directory. Execute 'make build' first"
	@echo "uninstall"
	@echo "      Uninstalls the executable from /usr/local/bin directory"
	@echo "format"
	@echo "      Run clang-format."
	@echo "format-dry-run"
	@echo "      Run clang-format with --dry-run --Werror flag."
	@echo "test"
	@echo "      Run tests, run make build DEBUG=1 first"
	@echo "coverage"
	@echo "      Run test and prepare code coverage report, run make build GCOV=1"
	@echo "gen"
	@echo "      Generate test cases, please review the generated test cases using git diff after running this"
	@echo "tidy"
	@echo "      Run clang-tidy"
	@echo ""

# Name of the executable
EXEC_NAME = shtkc

# C++ compiler
CXX ?= clang++

# Set version string
VERSION_STRING = $(shell cat version)

# Get platform
ifeq ($(OS), Windows_NT)
	PLATFORM = $(OS)-$(PROCESSOR_ARCHITECTURE)
	EXEC_NAME := $(EXEC_NAME).exe
else ifeq ($(shell uname -s), Linux)
	DISTRO = $(shell grep -oP '(?<=^ID=).+' /etc/os-release | tr -d '"')
	PLATFORM = $(shell uname -s)-$(DISTRO)-$(shell uname -m)
else
	PLATFORM = $(shell uname -s)-$(shell uname -m)
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
	LDFLAGS = -pg
	BUILD_TYPE = gprof
else ifeq ($(GCOV), 1)
    LDFLAGS = --coverage

	ifneq ($(shell uname -s), Darwin)
		LDFLAGS := $(LDFLAGS) -lgcov
	endif
	
	CXXFLAGS = -fprofile-arcs -ftest-coverage -g
	BUILD_TYPE = gcov
else
    CXXFLAGS = -O3
	BUILD_TYPE = $(EXEC_NAME)-$(VERSION_STRING)-$(PLATFORM)
endif

# Get llvm-config bin
LLVM_CONFIG_12_BIN_EXISTS := $(shell llvm-config-12 --version >/dev/null 2>&1 || (echo "Does not exist"))

ifneq (,${LLVM_PATH})
	LLVM_CONFIG_BIN = $(LLVM_PATH)/bin/llvm-config
	LLVM_LLC_BIN = $(LLVM_PATH)/bin/llv
else ifeq (,${LLVM_CONFIG_12_BIN_EXISTS})
    LLVM_CONFIG_BIN = llvm-config-12
	LLVM_LLC_BIN = llc-12
else ifeq ($(shell uname -s), Darwin)

	ifeq ($(shell uname -m), arm64) 
		LLVM_BIN_PATH = /opt/homebrew/opt/llvm@12/bin
	else
		LLVM_BIN_PATH = /usr/local/opt/llvm@12/bin
	endif

	LLVM_CONFIG_BIN = $(LLVM_BIN_PATH)/llvm-config
	LLVM_LLC_BIN = $(LLVM_BIN_PATH)/llc
else
    LLVM_CONFIG_BIN = llvm-config
	LLVM_LLC_BIN = llc
endif

# Set compiler and linker flags for llvm
CXXFLAGS := $(CXXFLAGS) -I`$(LLVM_CONFIG_BIN) --includedir` --std=c++17  -Wall -DVERSION=\"$(VERSION_STRING)\"
LDFLAGS := $(LDFLAGS) `$(LLVM_CONFIG_BIN) --ldflags --system-libs --libs all`

# Find all .hpp files in compiler/src
HEADERS = $(shell find compiler/src -name '*.hpp')
# Find all .cpp files in compiler/src
SOURCES = $(shell find compiler/src -name '*.cpp')
# Set object file names, all object files are in obj/
OBJECTS = $(SOURCES:compiler/src/%.cpp=obj/$(BUILD_TYPE)/%.o)

# Creating directories required for linking and building executable
dirs:
	mkdir -p obj/$(BUILD_TYPE)/
	mkdir -p bin/$(BUILD_TYPE)/
	cd compiler/src/ && find . -type d -exec mkdir -p ../../obj/$(BUILD_TYPE)/{} \;

# For compiling .cpp files in compiler/src/ to .o object files in obj/
obj/$(BUILD_TYPE)/%.o: compiler/src/%.cpp $(HEADERS) 
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Linking all object files to executable 
bin/$(BUILD_TYPE)/$(EXEC_NAME): $(OBJECTS)
	$(CXX) -o bin/$(BUILD_TYPE)/$(EXEC_NAME) $(OBJECTS) $(LDFLAGS)
	rm -f build-name.txt
	touch build-name.txt
	echo $(BUILD_TYPE) >> build-name.txt

build: dirs bin/$(BUILD_TYPE)/$(EXEC_NAME)

install:
	cp bin/$(BUILD_TYPE)/$(EXEC_NAME) /usr/local/bin

uninstall:
	rm /usr/local/bin/$(EXEC_NAME)

format:
	clang-format -i $(SOURCES)

format-dry-run:
	clang-format -i $(SOURCES) --dry-run --Werror

test:
	LLC_BIN=$(LLVM_LLC_BIN) python3 -m tests_runner --test

coverage:
	LLC_BIN=$(LLVM_LLC_BIN) python3 -m tests_runner --coverage

gen:
	LLC_BIN=$(LLVM_LLC_BIN) python3 -m tests_runner --gen

tidy:
	clang-tidy $(SOURCES) -- $(CXXFLAGS) -Wextra

clean:
	rm -f -r bin/
	rm -f -r obj/
	rm -f -r compiler/tests/reports/
	rm -f -r compiler/tests/*.info
	rm -f compiler/tests/compiler/*.llc
	rm -f compiler/tests/compiler/*.llc.s
	rm -f compiler/tests/compiler/*.o
	rm -f compiler/tests/compiler/TestModules/*.o
	rm -f compiler/tests/compiler/test
	rm -f compiler/tests/compiler/test_executable
	rm -f compiler/tests/*.gmon.out*
	rm -f stdlib/tests/*.o
	rm -f stdlib/tests/test_executable
	rm -f stdlib/tests/test
	rm -f stdlib/stdlib/*.o
	rm -rf .mypy_cache
	find . -type d -name  "__pycache__" -exec rm -r {} +
	find . -type d -name  "_obj" -exec rm -r {} +
	rm -f -r $(BUILD_TYPE).AppDir/
	rm -f *.AppImage
	rm -f *.tar.xz
	rm -rf llvm
	rm -f build-name.txt
	rm -rf appimage-builder-cache
	rm -rf AppDir
