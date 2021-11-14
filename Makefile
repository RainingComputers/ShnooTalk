.PHONY : help build install install-gedit uninstall format format-dry-run test coverage tidy clean appdir
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
	@echo "format-dry-run"
	@echo "      Run clang-format with --dry-run --Werror flags."
	@echo "test"
	@echo "      Run tests, run make build DEBUG=1 first"
	@echo "coverage"
	@echo "      Run test and prepare code coverage report, run make build GCOV=1"
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
else ifeq ($(CXX), x86_64-w64-mingw32-g++)
	PLATFORM = Windows_NT-AMD64
	EXEC_NAME := $(EXEC_NAME).exe
else ifeq ($(CXX), arm-linux-gnueabi-g++)
	PLATFORM = Linux-arm
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
    LDFLAGS = -lgcov --coverage
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
else ifeq (,${LLVM_CONFIG_12_BIN_EXISTS})
    LLVM_CONFIG_BIN = llvm-config-12
else ifeq ($(shell uname -s), Darwin)
	LLVM_CONFIG_BIN = /usr/local/opt/llvm@12/bin/llvm-config
else
    LLVM_CONFIG_BIN = llvm-config
endif

# Set compiler and linker flags for llvm
CXXFLAGS := $(CXXFLAGS) -I`$(LLVM_CONFIG_BIN) --includedir` --std=c++17  -Wall -DVERSION=\"$(VERSION_STRING)\"
LDFLAGS := $(LDFLAGS) `$(LLVM_CONFIG_BIN) --ldflags --system-libs --libs all` -ls

# Find all .hpp files in src/
HEADERS = $(shell find src/ -name '*.hpp')
# Find all .cpp files in src/
SOURCES = $(shell find src/ -name '*.cpp')
# Set object file names, all object files are in obj/
OBJECTS = $(SOURCES:src/%.cpp=obj/$(BUILD_TYPE)/%.o)

# Creating directories required for linking and building executable
dirs:
	mkdir -p obj/$(BUILD_TYPE)/
	mkdir -p bin/$(BUILD_TYPE)/
	cd src/ && find . -type d -exec mkdir -p ../obj/$(BUILD_TYPE)/{} \;

# For compiling .cpp files in src/ to .o object files in obj/
obj/$(BUILD_TYPE)/%.o: src/%.cpp $(HEADERS) 
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

install-gedit:
	cp shnootalk.lang /usr/share/gtksourceview-4/language-specs/shnootalk.lang

uninstall:
	rm /usr/local/bin/$(EXEC_NAME)

format:
	clang-format -i $(SOURCES)

format-dry-run:
	clang-format -i $(SOURCES) --dry-run --Werror

test:
	python3 -m tests_runner --test

coverage:
	python3 -m tests_runner --coverage

tidy:
	clang-tidy src/*.cpp src/*/*.cpp  -- $(CXXFLAGS) -Wextra

clean:
	rm -f -r bin/
	rm -f -r obj/
	rm -f -r tests/reports/
	rm -f -r tests/*.info
	rm -f tests/compiler/*.llc
	rm -f tests/compiler/*.llc.s
	rm -f tests/compiler/*.o
	rm -f tests/test
	rm -f tests/*.gmon.out*
	rm -rf .mypy_cache
	find . -type d -name  "__pycache__" -exec rm -r {} +
	rm -f -r $(BUILD_TYPE).AppDir/
	rm -f *.AppImage
	rm -f *.tar.xz
	rm -rf llvm
	rm -f build-name.txt
	rm -rf AppDir
	rm -rf appimage-builder-cache
