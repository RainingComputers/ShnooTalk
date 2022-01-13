
import os
import json

from tests_runner.framework import TestResult
from tests_runner.framework import compile_phase, string_validator
from tests_runner.framework import string_from_file
from tests_runner.framework import batch_run


def run_single_ir(file_name: str) -> TestResult:
    expected_output = string_from_file(os.path.join("expected/json", file_name+".json"))

    try:
        json.loads(expected_output)
    except json.decoder.JSONDecodeError:
        return TestResult.invalid(expected_output)

    return string_validator(
        compile_phase_result=compile_phase(
            file_name=file_name,
            compile_flag="-json-ir",
            compiler_output_dump_file=None,
            create_executable=False,
            skip_on_compile_error=False
        ),
        expected_output_on_success=expected_output
    )


def run() -> None:
    os.chdir("tests/ir")

    batch_run(
        "IR JSON",
        run_single_ir
    )

    os.chdir("../..")
