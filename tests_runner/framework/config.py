import os
import sys
import enum

from tests_runner.framework.fs import string_from_file


class CLIArg(str, enum.Enum):
    TEST = "--test"
    COVERAGE = "--coverage"


class BuildType(str, enum.Enum):
    DEBUG = "debug"
    GCOV = "gcov"


BUILD_TYPE_MAP = {
    CLIArg.TEST: BuildType.DEBUG,
    CLIArg.COVERAGE: BuildType.GCOV,
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

# Find compiler

COMPILER_EXEC_PATH = os.path.join(os.getcwd(), f"bin/{BUILD_TYPE}/shtkc")
COMPILER_NOT_FOUND = True

if os.path.exists(COMPILER_EXEC_PATH):
    COMPILER_NOT_FOUND = False
else:
    print(f"🙁 compiler not found at {COMPILER_EXEC_PATH}")

# Setup other constants

LLC_BIN = os.getenv("LLC_BIN", default="llc-12")

OBJ_DIR = os.path.join(os.getcwd(),  f"./obj/{BUILD_TYPE}/")

COVERAGE_REPORT_DIR = os.path.join(os.getcwd(), "tests",  "reports")

VERSION_FILE = os.path.join(os.getcwd(), "version")

COMPILER_VERSION = string_from_file(VERSION_FILE)

TIMEOUT = 5

OPEN_BIN = 'open' if sys.platform.title() == 'Darwin' else 'xdg-open'

LCOV_EXCLUDE_LIST = [
    '/usr/include/*',
    '/usr/lib/*',
    '/Library/*',
    '/opt/*'
]

LCOV_EXCLUDE = ' '.join(list(map(lambda x: f"'{x}'", LCOV_EXCLUDE_LIST)))
