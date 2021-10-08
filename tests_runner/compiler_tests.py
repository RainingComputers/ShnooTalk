import os

from tests_runner.config import BUILD_TYPE, COMPILER_EXEC_PATH

from tests_runner.util.run_subprocess import run_subprocess
from tests_runner.util.test_dir import list_test_files, get_files
from tests_runner.util.test_dir import remove_files, remove_test_executable
from tests_runner.util.result import TestResult, ResultPrinter
from tests_runner.util.compare import compare_outputs
from tests_runner.coverage import prepare_coverage_report, setup_coverage_dir


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


def run_single(file_name: str, compiler_exec_path: str) -> TestResult:
    expected_output = get_expected_output(file_name)

    # Remove all object files before running the test
    remove_files(".o")
    remove_test_executable()

    # Set profile output file name (applicable if coverage)
    os.environ["GMON_OUT_PREFIX"] = file_name + ".gmon.out"

    # Run the compiler
    compile_command = [compiler_exec_path, file_name, "-c"]
    timedout, compiler_output, compiler_retcode = run_subprocess(compile_command)

    if timedout:
        return TestResult.timedout()

    # If there was a compilation error
    if compiler_retcode != 0:
        return compare_outputs(expected_output, compiler_output)

    # Link object file into an executable
    object_files = " ".join(get_files(".o"))
    os.system(f"gcc {object_files} -o test -lm")

    # Run the executable
    timedout, exec_output, _ = run_subprocess(["./test"])

    if timedout:
        return TestResult.timedout()

    # If the program/executable did not timeout, return program output
    return compare_outputs(expected_output, exec_output)


def run() -> None:
    setup_coverage_dir()

    result_printer = ResultPrinter("ShnooTalk compiler")

    for file in list_test_files():
        test_result = run_single(file, COMPILER_EXEC_PATH)
        result_printer.print_result(file, test_result)

    result_printer.print_summary()

    if BUILD_TYPE == "gcov":
        prepare_coverage_report(result_printer.passed)
