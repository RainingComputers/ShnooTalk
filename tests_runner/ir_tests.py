
import os

from tests_runner.framework import Result
from tests_runner.framework import compile_phase, compile_success_output_assert
from tests_runner.framework import tester


@tester.batch("compiler/tests/ir")
def ir_json(file_name: str) -> Result:
    test_case_file_path = os.path.join("expected/json", file_name+".json")

    return compile_success_output_assert(
        compile_phase_result=compile_phase(
            file_name=file_name,
            compile_flag="-json-ir-all",
            compiler_output_dump_file=None,
            create_executable=False,
            skip_on_compile_error=False
        ),
        expected_test_case_file=test_case_file_path,
        check_json=True
    )
