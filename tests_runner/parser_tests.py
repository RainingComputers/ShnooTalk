
import os
import json

from tests_runner.utils.result import TestResult
from tests_runner.utils.validator import string_validator
from tests_runner.utils.dir import string_from_file
from tests_runner.utils.batch import batch_run


def run_single_pretty(file_name: str) -> TestResult:
    expected_output = string_from_file(os.path.join("expected/pretty", file_name+".txt"))

    return string_validator(
        file_name=file_name,
        flag="-ast",
        expected=expected_output
    )


def run_single_json(file_name: str) -> TestResult:
    expected_output = string_from_file(os.path.join("expected/json", file_name+".json"))

    try:
        json.loads(expected_output)
    except json.decoder.JSONDecodeError:
        return TestResult.invalid(expected_output)

    return string_validator(
        file_name=file_name,
        flag="-json-ast",
        expected=expected_output
    )


def run() -> None:
    os.chdir("tests/parser")

    batch_run(
        "Parser AST pretty",
        run_single_pretty
    )

    batch_run(
        "Parser AST JSON",
        run_single_json
    )

    os.chdir("../..")
