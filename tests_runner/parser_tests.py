
import os

from tests_runner.framework import TestResult
from tests_runner.framework import compile_phase, compiler_output_assert
from tests_runner.framework import string_from_file, json_from_file
from tests_runner.framework import batch_run


def run_single_pretty(file_name: str) -> TestResult:
    expected_output = string_from_file(os.path.join("expected/pretty", file_name+".txt"))

    return compiler_output_assert(
        compile_phase_result=compile_phase(
            file_name=file_name,
            compile_flag="-ast",
            compiler_output_dump_file=None,
            create_executable=False,
            skip_on_compile_error=False
        ),
        expected_output_on_success=expected_output
    )


def run_single_json(file_name: str) -> TestResult:
    expected_output = json_from_file(os.path.join("expected/json", file_name+".json"))

    return compiler_output_assert(
        compile_phase_result=compile_phase(
            file_name=file_name,
            compile_flag="-json-ast",
            compiler_output_dump_file=None,
            create_executable=False,
            skip_on_compile_error=False
        ),
        expected_output_on_success=expected_output
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
