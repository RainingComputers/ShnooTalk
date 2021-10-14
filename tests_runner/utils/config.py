from typing import Optional

import sys


ARG_MAP = {
    "--coverage": "gcov",
    "--profile": "gprof",
    "--gen-ast": "ast_gen",
    "--gen-json-ast": "ast_json_gen",
    "--gen-ir": "ir_gen",
    "--gen-json-ir": "ir_json_gen"
}


def parse_args() -> Optional[str]:
    if len(sys.argv) == 1:
        return "debug"

    if len(sys.argv) > 2:
        return None

    try:
        return ARG_MAP[sys.argv[1]]
    except KeyError:
        return None


CLI_ARG = parse_args()

if CLI_ARG is None:
    COMPILER_EXEC_PATH = None
elif CLI_ARG[-4:] != "_gen":
    COMPILER_EXEC_PATH = f"../bin/{CLI_ARG}/shtkc"
else:
    COMPILER_EXEC_PATH = "../bin/debug/shtkc"

OBJ_DIR = f"../obj/{CLI_ARG}/"

COVERAGE_INFO_DIR = "testinfo"

TIMEOUT = 5
