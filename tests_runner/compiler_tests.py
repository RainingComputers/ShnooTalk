import os

from tests_runner.framework import Result
from tests_runner.framework import compile_phase
from tests_runner.framework import command_on_compile_success_output_assert
from tests_runner.framework import compile_success_output_assert
from tests_runner.framework import tester


@tester.batch("compiler/tests/compiler")
def output_executable(file_name: str) -> Result:
    test_case_file_path = os.path.join("expected/output", file_name)+".txt"

    return command_on_compile_success_output_assert(
        compile_phase_result=compile_phase(
            file_name=file_name,
            compile_flag="-c",
            compiler_output_dump_file=None,
            create_executable=True,
            skip_on_compile_error=False,
        ),
        compile_result_fail_test_case_file=test_case_file_path,
        command_on_compile_result_pass=["./test_executable"],
        command_ignore_non_zero_exit_code=False,
        command_output_test_case_file=test_case_file_path
    )


@tester.batch("compiler/tests/compiler")
def icode_pretty(file_name: str) -> Result:
    test_case_file_path = os.path.join("expected/pretty", file_name)+".txt"

    return compile_success_output_assert(
        compile_phase_result=compile_phase(
            file_name=file_name,
            compile_flag="-icode-all",
            compiler_output_dump_file=None,
            create_executable=False,
            skip_on_compile_error=True,
        ),
        expected_test_case_file=test_case_file_path,
        check_json=False
    )


@tester.batch("compiler/tests/compiler")
def icode_json(file_name: str) -> Result:
    test_case_file_path = os.path.join("expected/json", file_name)+".json"

    return compile_success_output_assert(
        compile_phase_result=compile_phase(
            file_name=file_name,
            compile_flag="-json-icode-all",
            compiler_output_dump_file=None,
            create_executable=False,
            skip_on_compile_error=True,
        ),
        expected_test_case_file=test_case_file_path,
        check_json=True
    )
