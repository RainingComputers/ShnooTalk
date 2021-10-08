# pylint: disable=too-many-arguments
# pylint: disable=too-many-return-statements

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


def cleanup() -> None:
    remove_if_exists("./test_executable")


def dump_string_to_file(file_name: str, content: str) -> None:
    with open(file_name, 'w') as file:
        file.write(content)


def compare(expected_output: Optional[str], output: str) -> TestResult:
    if expected_output == output:
        return TestResult.passed()

    return TestResult.failed(output, expected_output)


def phase_executer(file_name: str,
                   compile_flag: str,
                   compiler_output_dump_file: Optional[str],
                   command: List[str],
                   link_phase: bool,
                   skip_on_compile_error: bool,
                   expected: Optional[str] = None) -> None:
    setup(file_name)

    # Run the shnootalk compiler
    compile_timedout, compiler_output, compiler_exit_code = shtk_compile(file_name, compile_flag)

    if compile_timedout:
        return TestResult.timedout()

    if compiler_exit_code != 0:
        if skip_on_compile_error:
            return TestResult.skipped()

        return compare(expected, compiler_output)

    if compiler_output_dump_file is not None:
        dump_string_to_file(compiler_output_dump_file, compiler_output)

    # Link objects if necessary
    if link_phase:
        link_objects()

    # Run the given command
    command_timedout, command_output, command_exit_code = run_command(command)

    cleanup()

    if command_timedout:
        return TestResult.timedout()

    if expected is not None:
        return compare(expected, command_output)

    if command_exit_code != 0:
        return TestResult.failed(command_output)

    return TestResult.passed()
