import os

from tests_runner.framework import TestResult
from tests_runner.framework import compile_phase
from tests_runner.framework import command_on_compile_success_output_assert
from tests_runner.framework import compile_success_output_assert
from tests_runner.framework import batch_run


def get_expected_output(file_name: str) -> str:
    # Extract commented test case from beginning of the file
    expected_output = ""

    with open(file_name) as test_program:
        while True:
            line = next(test_program)
            if line[0] != "#":
                break

            expected_output += line[2:]

    return expected_output


def run_single_exec(file_name: str) -> TestResult:
    expected_output = get_expected_output(file_name)

    return command_on_compile_success_output_assert(
        compile_phase_result=compile_phase(
            file_name=file_name,
            compile_flag="-c",
            compiler_output_dump_file_path=None,
            create_executable=True,
            skip_on_compile_error=False,
        ),
        expected_on_compile_result_fail=expected_output,
        command_on_compile_result_pass=["./test_executable"],
        expected_command_output=expected_output
    )


def run_single_icode_pretty(file_name: str) -> TestResult:
    test_case_file = os.path.join("expected/pretty", file_name)+".txt"

    return compile_success_output_assert(
        compile_phase_result=compile_phase(
            file_name=file_name,
            compile_flag="-icode",
            compiler_output_dump_file_path=None,
            create_executable=False,
            skip_on_compile_error=True,
        ),
        expected_test_case_file_path=test_case_file,
        check_json=False
    )


def run_single_icode_json(file_name: str) -> TestResult:
    test_case_file = os.path.join("expected/json", file_name)+".json"

    return compile_success_output_assert(
        compile_phase_result=compile_phase(
            file_name=file_name,
            compile_flag="-json-icode",
            compiler_output_dump_file_path=None,
            create_executable=False,
            skip_on_compile_error=True,
        ),
        expected_test_case_file_path=test_case_file,
        check_json=True
    )


def run() -> None:
    os.chdir("tests/compiler")

    batch_run("Compiler output executable", run_single_exec)

    batch_run("Compiler output icode pretty", run_single_icode_pretty)

    batch_run("Compiler output icode JSON", run_single_icode_json)

    os.chdir("../..")
