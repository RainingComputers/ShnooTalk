import os
import sys
import enum

from tests_runner.framework.fs import string_from_file


class CLIArg(str, enum.Enum):
    TEST = "--test"
    COVERAGE = "--coverage"
    GEN = "--gen"


class BuildType(str, enum.Enum):
    DEBUG = "debug"
    GCOV = "gcov"


BUILD_TYPE_MAP = {
    CLIArg.TEST: BuildType.DEBUG,
    CLIArg.COVERAGE: BuildType.GCOV,
    CLIArg.GEN: BuildType.DEBUG
}

CLI_ARG_OPTIONS = ' '.join(list(BUILD_TYPE_MAP.keys()))

# Parse CLI arguments

CLI_ARG = None
BUILD_TYPE = None
INVALID_CLI_ARGS = True

if len(sys.argv) == 2:
    try:
        CLI_ARG = CLIArg(sys.argv[1])
        BUILD_TYPE = BUILD_TYPE_MAP[CLI_ARG]
        INVALID_CLI_ARGS = False
    except ValueError:
        print("🙁 Invalid CLI ARGS, available option are:")
        print(f"    {CLI_ARG_OPTIONS}")

# Get compiler version

VERSION_FILE = os.path.join(os.getcwd(), "version")
COMPILER_VERSION = string_from_file(VERSION_FILE)

# Set coverage report directory, coverage exclude list and executable to open html report

COVERAGE_REPORT_DIR = os.path.join(os.getcwd(), "tests",  "reports")

LCOV_EXCLUDE_LIST = [
    '/usr/include/*',
    '/usr/lib/*',
    '/Library/*',
    '/opt/*'
]

LCOV_EXCLUDE = ' '.join(list(map(lambda x: f"'{x}'", LCOV_EXCLUDE_LIST)))

OPEN_BIN = 'open' if sys.platform.title() == 'Darwin' else 'xdg-open'

# Setup other constants related to test cases

LLC_BIN = os.getenv("LLC_BIN", default="llc-12")

OBJ_DIR = os.path.join(os.getcwd(),  f"./obj/{BUILD_TYPE}/")

TIMEOUT = 5

# Build the compiler

BUILD_COMMAND_MAP = {
    BuildType.DEBUG: os.getenv("DEBUG_BUILD_COMMAND", "make build DEBUG=1 CXX=ccache\\ g++ -j 4"),
    BuildType.GCOV: os.getenv("COVERAGE_BUILD_COMMAND", "make build GCOV=1 CXX=ccache\\ g++ -j 4")
}

COMPILER_BUILD_FAILED = True

if BUILD_TYPE is not None:
    print("🤖 Building compiler...")
    exit_code = os.system(BUILD_COMMAND_MAP[BUILD_TYPE])
    COMPILER_BUILD_FAILED = exit_code != 0

# Find compiler

COMPILER_EXEC_PATH = os.path.join(os.getcwd(), f"bin/{BUILD_TYPE}/shtkc")
COMPILER_NOT_FOUND = True

if COMPILER_BUILD_FAILED:
    print("🙁 compiler build failed")
if os.path.exists(COMPILER_EXEC_PATH):
    COMPILER_NOT_FOUND = False
else:
    print(f"🙁 compiler not found at {COMPILER_EXEC_PATH}")
