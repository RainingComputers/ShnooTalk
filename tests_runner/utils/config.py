from typing import Optional

import sys


BUILD_TYPE_MAP = {
    "--core": "debug",
    "--test": "debug",
    "--coverage": "gcov",
    "--gen-ast": "debug",
    "--gen-ir": "debug",
}


def parse_args() -> Optional[str]:
    if len(sys.argv) != 2:
        return None

    try:
        return BUILD_TYPE_MAP[sys.argv[1]]
    except KeyError:
        return None


BUILD_TYPE = parse_args()

CLI_ARG = None if BUILD_TYPE is None else sys.argv[1]

COMPILER_EXEC_PATH = f"../bin/{BUILD_TYPE}/shtkc"

OBJ_DIR = f"../obj/{BUILD_TYPE}/"

COVERAGE_INFO_DIR = "testinfo"

TIMEOUT = 5
