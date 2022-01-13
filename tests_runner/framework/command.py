from typing import Tuple, List, Optional

import subprocess

from tests_runner.framework.config import TIMEOUT


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
