from typing import List, Optional, Tuple

from tests_runner.config import COMPILER_EXEC_PATH

from tests_runner.util.command import run_command
from tests_runner.util.dir import get_files, remove_if_exists
from tests_runner.util.dir import remove_files
from tests_runner.util.result import TestResult
from tests_runner.util.coverage import set_gmon_prefix, setup_coverage_dir


def setup(file_name: str) -> None:
    # Remove all object files before running the test
    remove_files(".o")
    remove_if_exists("./test_executable")

    # Set profile output file name (applicable if lcov build)
    set_gmon_prefix(file_name)

    # Create dir for string .info files (lcov)
    setup_coverage_dir()


def shtk_compile(file_name: str, compile_flag: str) -> Tuple[bool, str, Optional[int]]:
    return run_command([COMPILER_EXEC_PATH, file_name, compile_flag])


def link_objects() -> None:
    run_command(["gcc"] + get_files(".o") + ["-o", "test_executable", "-lm"])


def dump_string_to_file(file_name: str, content: str) -> None:
    with open(file_name, 'w') as file:
        file.write(content)


def compare(expected_output: str, output: str) -> TestResult:
    if expected_output == output:
        return TestResult.passed()

    return TestResult.failed(output, expected_output)


def compile_phase(file_name: str,
                  compile_flag: str,
                  compiler_output_dump_file: Optional[str],
                  link_phase: bool,
                  skip_on_compile_error: bool) -> TestResult:
    setup(file_name)

    # Run the shnootalk compiler
    compile_timedout, compiler_output, compiler_exit_code = shtk_compile(file_name, compile_flag)

    if compile_timedout:
        return TestResult.timedout()

    if compiler_exit_code != 0:
        if skip_on_compile_error:
            return TestResult.skipped()

        return TestResult.failed(compiler_output)

    if compiler_output_dump_file is not None:
        dump_string_to_file(compiler_output_dump_file, compiler_output)

    # Link objects if necessary
    if link_phase:
        link_objects()

    return TestResult.passed()


def validate(compile_phase_result: TestResult,
             expected_on_compile_fail: Optional[str],
             command_on_compile_success: List[str],
             expected_command_output: Optional[str]) -> None:

    if compile_phase_result.has_failed and expected_on_compile_fail is not None:
        return compare(expected_on_compile_fail, compile_phase_result.output)

    if not compile_phase_result.has_passed:
        return compile_phase_result

    command_timedout, command_output, command_exit_code = run_command(command_on_compile_success)

    remove_if_exists("./test_executable")

    if command_timedout:
        return TestResult.timedout()

    if expected_command_output is not None:
        return compare(expected_command_output, command_output)

    if command_exit_code != 0:
        return TestResult.failed(command_output)

    return TestResult.passed()
