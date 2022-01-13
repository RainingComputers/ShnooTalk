from typing import Optional

import os
import sys

from tests_runner.framework.fs import string_from_file

BUILD_TYPE_MAP = {
    "--test": "debug",
    "--coverage": "gcov",
}

CLI_ARG_OPTIONS = ' '.join(list(BUILD_TYPE_MAP.keys()))


def parse_args() -> Optional[str]:
    if len(sys.argv) != 2:
        return None

    try:
        return BUILD_TYPE_MAP[sys.argv[1]]
    except KeyError:
        return None


PARSED_ARGS = parse_args()

CLI_ARG = None if PARSED_ARGS is None else sys.argv[1]

BUILD_TYPE = "debug" if PARSED_ARGS is None else PARSED_ARGS

COMPILER_EXEC_PATH = os.path.join(os.getcwd(),  f"bin/{BUILD_TYPE}/shtkc")

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
