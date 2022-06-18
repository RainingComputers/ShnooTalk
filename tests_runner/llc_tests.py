from tests_runner.framework import Result
from tests_runner.framework import LLC_BIN
from tests_runner.framework import compile_phase, command_on_compile_success_output_assert
from tests_runner.framework import tester


@tester.batch("compiler/tests/compiler", [".o", ".llc", ".llc.s"])
def llc(file_name: str) -> Result:
    llc_file_name = file_name + ".llc"

    return command_on_compile_success_output_assert(
        compile_phase_result=compile_phase(
            file_name=file_name,
            compile_flag="-llvm",
            compiler_output_dump_file=llc_file_name,
            create_executable=False,
            skip_on_compile_error=True
        ),
        compile_result_fail_test_case_file=None,
        command_on_compile_result_pass=[LLC_BIN, llc_file_name],
        command_ignore_non_zero_exit_code=False,
        command_output_test_case_file=None,
    )
