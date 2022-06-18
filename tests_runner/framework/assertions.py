from typing import List, Optional

import json

from tests_runner.framework.config import COMPILER_EXEC_PATH

from tests_runner.framework.command import run_command, link_objects_into_bin, shtk_compile
from tests_runner.framework.fs import remove_if_exists, dump_string_to_file, remove_files
from tests_runner.framework.fs import string_from_file
from tests_runner.framework.result import Result
from tests_runner.framework.coverage import set_gmon_prefix


def setup(file_name: str) -> None:
    # Remove all object files before running the test
    remove_files(".o")
    remove_if_exists("./test_executable")

    # Set profile output file name (applicable if lcov build)
    set_gmon_prefix(file_name)


def compare(expected_output: str, output: Optional[str]) -> Result:
    if output is None:
        if expected_output == "":
            return Result.passed("")

        return Result.failed("", expected_output)

    if expected_output == output:
        return Result.passed(output)

    return Result.failed(output, expected_output)


def compile_phase(file_name: str,
                  compile_flag: str,
                  compiler_output_dump_file: Optional[str],
                  create_executable: bool,
                  skip_on_compile_error: bool) -> Result:
    setup(file_name)

    # Run the shnootalk compiler
    compile_timedout, compiler_output, compiler_exit_code = shtk_compile(file_name, compile_flag)

    if compile_timedout:
        return Result.timedout()

    if compiler_exit_code != 0:
        if skip_on_compile_error:
            return Result.skipped()

        return Result.failed(compiler_output)

    if compiler_output_dump_file is not None:
        dump_string_to_file(compiler_output_dump_file, compiler_output)

    if create_executable:
        link_objects_into_bin()

    return Result.passed(compiler_output)


def command_on_compile_success_output_assert(
    compile_phase_result: Result,
    compile_result_fail_test_case_file: Optional[str],
    command_on_compile_result_pass: List[str],
    command_ignore_non_zero_exit_code: bool,
    command_output_test_case_file: Optional[str]
) -> Result:

    if compile_phase_result.has_failed and compile_result_fail_test_case_file is not None:
        expected = string_from_file(compile_result_fail_test_case_file)
        return compare(expected, compile_phase_result.output)

    if not compile_phase_result.has_passed:
        return compile_phase_result

    command_timedout, command_output, command_exit_code = \
        run_command(command_on_compile_result_pass)

    remove_if_exists("./test_executable")

    if command_timedout:
        return Result.timedout()

    if command_exit_code != 0 and not command_ignore_non_zero_exit_code:
        return Result.failed(command_output)

    if command_output_test_case_file is not None:
        expected = string_from_file(command_output_test_case_file)
        return compare(expected, command_output)

    return Result.passed(command_output)


def compile_success_output_assert(compile_phase_result: Result,
                                  expected_test_case_file: str,
                                  check_json: bool) -> Result:

    if not compile_phase_result.has_passed:
        return compile_phase_result

    expected_output = string_from_file(expected_test_case_file)

    if check_json:
        _ = json.loads(expected_output)

    return compare(expected_output, compile_phase_result.output)


def simple_output_assert(compiler_args: List[str], expected_output: str,
                         expect_non_zero_exit_code: bool) -> Result:
    cmd = [COMPILER_EXEC_PATH] + compiler_args

    timedout, output, exit_code = run_command(cmd)

    if exit_code == 0 and expect_non_zero_exit_code:
        return Result.failed(output, expected_output)

    if timedout:
        return Result.failed(output, expected_output)

    return compare(expected_output, output)
