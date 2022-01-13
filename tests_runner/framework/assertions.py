from typing import List, Optional, Tuple

import json

from tests_runner.framework.config import COMPILER_EXEC_PATH

from tests_runner.framework.command import run_command
from tests_runner.framework.fs import get_files, remove_if_exists, dump_string_to_file, remove_files
from tests_runner.framework.fs import string_from_file
from tests_runner.framework.result import TestResult
from tests_runner.framework.coverage import set_gmon_prefix, setup_coverage_dir


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


def link_objects_into_bin() -> None:
    run_command(["gcc"] + get_files(".o") + ["-o", "test_executable", "-lm"])


def compare(expected_output: str, output: Optional[str]) -> TestResult:
    if output is None:
        if expected_output == "":
            return TestResult.passed("")

        return TestResult.failed("", expected_output)

    if expected_output == output:
        return TestResult.passed(output)

    return TestResult.failed(output, expected_output)


def compile_phase(file_name: str,
                  compile_flag: str,
                  compiler_output_dump_file_path: Optional[str],
                  create_executable: bool,
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

    if compiler_output_dump_file_path is not None:
        dump_string_to_file(compiler_output_dump_file_path, compiler_output)

    if create_executable:
        link_objects_into_bin()

    return TestResult.passed(compiler_output)


def command_on_compile_success_output_assert(compile_phase_result: TestResult,
                                             expected_on_compile_result_fail: Optional[str],
                                             command_on_compile_result_pass: List[str],
                                             expected_command_output: Optional[str]) -> TestResult:

    if compile_phase_result.has_failed and expected_on_compile_result_fail is not None:
        return compare(expected_on_compile_result_fail, compile_phase_result.output)

    if not compile_phase_result.has_passed:
        return compile_phase_result

    command_timedout, command_output, command_exit_code = \
        run_command(command_on_compile_result_pass)

    remove_if_exists("./test_executable")

    if command_timedout:
        return TestResult.timedout()

    if command_exit_code != 0:
        return TestResult.failed(command_output)

    if expected_command_output is not None:
        return compare(expected_command_output, command_output)

    return TestResult.passed(command_output)


def compile_success_output_assert(compile_phase_result: TestResult,
                                  expected_test_case_file_path: str,
                                  check_json: bool) -> TestResult:

    if not compile_phase_result.has_passed:
        return compile_phase_result

    expected_output = string_from_file(expected_test_case_file_path)

    if check_json:
        _ = json.loads(expected_output)

    return compare(expected_output, compile_phase_result.output)


def simple_output_assert(compiler_args: List[str], expected_output: str,
                         expect_non_zero_exit_code: bool) -> TestResult:
    cmd = [COMPILER_EXEC_PATH] + compiler_args

    timedout, output, exit_code = run_command(cmd)

    if exit_code == 0 and expect_non_zero_exit_code:
        return TestResult.failed(output, expected_output)

    if timedout:
        return TestResult.failed(output, expected_output)

    return compare(expected_output, output)
