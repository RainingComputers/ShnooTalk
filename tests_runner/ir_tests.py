
import os

from tests_runner.framework import TestResult
from tests_runner.framework import compile_phase, compile_success_output_assert
from tests_runner.framework import tester


@tester.batch("ir json", "tests/ir")
def run_single_ir(file_name: str) -> TestResult:
    test_case_file_path = os.path.join("expected/json", file_name+".json")

    return compile_success_output_assert(
        compile_phase_result=compile_phase(
            file_name=file_name,
            compile_flag="-json-ir",
            compiler_output_dump_file_path=None,
            create_executable=False,
            skip_on_compile_error=False
        ),
        expected_test_case_file_path=test_case_file_path,
        check_json=True
    )


def register() -> None:
    # pylint: disable=no-value-for-parameter
    run_single_ir()
