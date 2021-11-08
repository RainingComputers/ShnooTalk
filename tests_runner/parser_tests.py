
import os
import json

from tests_runner.utils.result import TestResult
from tests_runner.utils.validator import compile_phase, string_validator
from tests_runner.utils.dir import string_from_file
from tests_runner.utils.batch import batch_run


def run_single_pretty(file_name: str) -> TestResult:
    expected_output = string_from_file(os.path.join('expected/pretty', file_name+'.txt'))

    return string_validator(
        compile_phase_result=compile_phase(
            file_name=file_name,
            compile_flag='-ast',
            compiler_output_dump_file=None,
            create_executable=False,
            skip_on_compile_error=False
        ),
        expected_output_on_success=expected_output
    )


def run_single_json(file_name: str) -> TestResult:
    expected_output = string_from_file(os.path.join('expected/json', file_name+'.json'))

    try:
        json.loads(expected_output)
    except json.decoder.JSONDecodeError:
        return TestResult.invalid(expected_output)

    return string_validator(
        compile_phase_result=compile_phase(
            file_name=file_name,
            compile_flag='-json-ast',
            compiler_output_dump_file=None,
            create_executable=False,
            skip_on_compile_error=False
        ),
        expected_output_on_success=expected_output
    )


def run() -> None:
    os.chdir('tests/parser')

    batch_run(
        'Parser AST pretty',
        run_single_pretty
    )

    batch_run(
        'Parser AST JSON',
        run_single_json
    )

    os.chdir('../..')
