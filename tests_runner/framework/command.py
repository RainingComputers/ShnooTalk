from typing import Tuple, List, Optional

import subprocess

from tests_runner.framework.fs import get_files
from tests_runner.framework.config import TIMEOUT, COMPILER_EXEC_PATH


def run_command(command: List[str]) -> Tuple[bool, str, Optional[int]]:
    try:
        subp = subprocess.run(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=TIMEOUT
        )

        console_output = subp.stdout.decode("utf-8")
        console_err_output = subp.stderr.decode("utf-8")

    except subprocess.TimeoutExpired:
        return True, "", None

    return False, console_output+console_err_output, subp.returncode


def shtk_compile(file_name: str, compile_flag: str) -> Tuple[bool, str, Optional[int]]:
    return run_command([COMPILER_EXEC_PATH, file_name, compile_flag])


def link_objects_into_bin() -> None:
    run_command(["gcc"] + get_files(".o") + ["-o", "test_executable", "-lm"])
