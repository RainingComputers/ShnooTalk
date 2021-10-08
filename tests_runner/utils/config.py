from typing import Optional

import sys


def parse_args() -> Optional[str]:
    if len(sys.argv) == 1:
        return "debug"

    if len(sys.argv) > 2:
        return None

    if sys.argv[1] == "--coverage":
        return "gcov"

    if sys.argv[1] == "--profile":
        return "gprof"

    return None


BUILD_TYPE = parse_args()

COMPILER_EXEC_PATH = f"../bin/{BUILD_TYPE}/shtkc"

OBJ_DIR = f"../obj/{BUILD_TYPE}/"

COVERAGE_INFO_DIR = "testinfo"

TIMEOUT = 5
