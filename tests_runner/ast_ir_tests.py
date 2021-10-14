from typing import Callable, Optional

import os

from tests_runner.utils.config import CLI_ARG

from tests_runner.utils.result import TestResult
from tests_runner.utils.validator import string_validator
from tests_runner.utils.dir import string_from_file
from tests_runner.utils.batch import batch_run


def get_expected_output(parent_dir: str, type_dir: str, file_name: str) -> Optional[str]:
    ext = ".json" if type_dir == "JSON" else ".txt"

    try:
        return string_from_file(os.path.join(parent_dir, type_dir, file_name+ext))
    except FileNotFoundError:
        return None


def run_single_config(parent_dir: str, type_dir: str, flag: str) -> Callable[[str], TestResult]:
    def run_single(file_name: str) -> TestResult:
        expected_output = get_expected_output(parent_dir, type_dir, file_name)

        if expected_output is None:
            return TestResult.skipped()

        return string_validator(
            file_name=file_name,
            flag=flag,
            expected=expected_output
        )

    return run_single


def run() -> None:
    batch_run(
        "AST pretty",
        run_single_config("ExpectedAST", "Pretty", "-ast"),
        CLI_ARG == "gcov"
    )

    batch_run(
        "AST JSON",
        run_single_config("ExpectedAST", "JSON", "-json-ast"),
        CLI_ARG == "gcov"
    )

    batch_run(
        "intermediate representation pretty",
        run_single_config("ExpectedIR", "Pretty", "-ir"),
        CLI_ARG == "gcov"
    )

    batch_run(
        "intermediate representation JSON",
        run_single_config("ExpectedIR", "JSON", "-json-ir"),
        CLI_ARG == "gcov"
    )
