from tests_runner.framework import TestResult
from tests_runner.framework import LLC_BIN
from tests_runner.framework import compile_phase, command_on_compile_success_output_assert
from tests_runner.framework import tester


@tester.batch("tests/compiler", [".o", ".llc", ".llc.s"])
def llc(file_name: str) -> TestResult:
    llc_file_name = file_name + ".llc"

    return command_on_compile_success_output_assert(
        compile_phase_result=compile_phase(
            file_name=file_name,
            compile_flag="-llvm",
            compiler_output_dump_file_path=llc_file_name,
            create_executable=False,
            skip_on_compile_error=True
        ),
        expected_on_compile_result_fail=None,
        command_on_compile_result_pass=[LLC_BIN, llc_file_name],
        expected_command_output=None,
    )


def register() -> None:
    # pylint: disable=no-value-for-parameter
    llc()
