.PHONY : help clean all build dirs install
help :
	@echo "clean"
	@echo "      Remove auto-generated files."
	@echo "build"
	@echo "      Build release executable."
	@echo "build CXX=x86_64-w64-mingw32-g++"
	@echo "      Cross-compile to build 64bit windows executable."
	@echo "build DEBUG=1"
	@echo "      Build executable for debugging."
	@echo "build GPROF=1"
	@echo "      Build executable for gprof."
	@echo "build GCOV=1"
	@echo "      Build executable for gcov."
	@echo "install"
	@echo "      Installs the executable to PATH. Execute 'make build' first."

# Name of the executable
EXEC_NAME = uhll

# C++ compiler
CXX ?= clang

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
	BUILD_TYPE = debug
else ifeq ($(GCOV), 1)
    LDFLAGS = -lgcov --coverage
    CXXFLAGS = -fprofile-arcs -ftest-coverage -g
	BUILD_TYPE = debug
else
    CXXFLAGS = -O3
	BUILD_TYPE = release_$(PLATFORM)
endif

CXXFLAGS := $(CXXFLAGS) `llvm-config-11 --cxxflags` -fexceptions
LDFLAGS := $(LDFLAGS) `llvm-config-11 --ldflags --system-libs --libs all`

# Find all .cpp files in src/
SOURCES = $(shell find src/ -name '*.cpp')
# Set object file names, all object files are in obj/
OBJECTS = $(SOURCES:src/%.cpp=obj/$(BUILD_TYPE)/%.o)

# Remove object files and executable
clean:
	rm -f -r bin/
	rm -f -r obj/
	rm -f -r tests/testinfo/
	rm -f -r .cccc/
	rm -f tests/*.llc
	rm -f tests/*.llc.s
	rm -f tests/*.o

# For compiling .cpp files in src/ to .o object files in obj/
obj/$(BUILD_TYPE)/%.o: src/%.cpp src/%.hpp
	$(CXX) $(CXXFLAGS) -Wall -c $< -o $@

obj/$(BUILD_TYPE)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -Wall -c $< -o $@

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

format:
	clang-format -i src/*.cpp
	clang-format -i src/*.hpp

quality:
	cccc src/*.cpp

install:
	cp bin/$(BUILD_TYPE)/$(EXEC_NAME) /usr/local/bin
	cp uhll.lang /usr/share/gtksourceview-4/language-specs/uhll.lang

uninstall:
	rm /usr/local/bin/$(EXEC_NAME)