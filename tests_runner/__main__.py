from typing import Tuple, Optional

import os
import sys

from tests_runner.util.run_subprocess import run_subprocess
from tests_runner.util.test_dir import list_test_files, get_files
from tests_runner.util.test_dir import remove_files, remove_test_executable
from tests_runner.util.result_printer import TestResultType, ResultPrinter
from tests_runner.util.coverage import prepare_coverage_report, setup_coverage_dir


def get_test_output(file_name: str) -> str:
    # Extract commented test case from beginning of the file
    test_output = ""

    with open(file_name) as test_program:
        while True:
            line = next(test_program)
            if line[0] != "#":
                break

            test_output += line[2:]

    return test_output


def compare_outputs(test_output: str, actual_output: Optional[str]) -> Tuple[int, str, str]:
    if test_output == actual_output or actual_output is None:
        return TestResultType.PASSED, "", test_output

    return TestResultType.FAILED, actual_output, test_output


def run_test(file_name: str, compiler_exec_path: str) -> Tuple[int, str, str]:
    test_output = get_test_output(file_name)

    # Remove all object files before running the test
    remove_files(".o")
    remove_test_executable()

    # Set profile output file name
    os.environ["GMON_OUT_PREFIX"] = file_name + ".gmon.out"

    # Run the compiler
    compile_command = [compiler_exec_path, file_name, "-c"]
    timedout, compiler_output, compiler_retcode = run_subprocess(compile_command)

    if timedout:
        return TestResultType.TIMEDOUT, "", test_output

    # If there was a compilation error, return the error message from the compiler
    if compiler_retcode != 0:
        return compare_outputs(test_output, compiler_output)

    # Link object file into an executable
    object_files = " ".join(get_files(".o"))
    os.system(f"gcc {object_files} -o test -lm")

    # Run the executable and return the output from the executable
    timedout, exec_output, _ = run_subprocess(["./test"])

    if timedout:
        return TestResultType.TIMEDOUT, "", test_output

    # If the program/executable did not timeout, return program output
    return compare_outputs(test_output, exec_output)


def run_test_llc(file_name: str, compiler_exec_path: str) -> Tuple[int, str]:
    # Run the compiler
    compile_command = [compiler_exec_path, file_name, "-llvm"]
    timedout, compiler_output, compiler_retcode = run_subprocess(compile_command)

    if timedout:
        return TestResultType.TIMEDOUT, ""

    if compiler_retcode != 0:
        return TestResultType.SKIPPED, ""

    # Run llc
    llc_file = file_name + ".llc"

    with open(llc_file, "w") as ll_f:
        ll_f.write(compiler_output)

    timedout, llc_output, llc_retcode = run_subprocess(["llc", llc_file])

    # Return test result
    if llc_retcode != 0:
        return TestResultType.FAILED, llc_output

    return TestResultType.PASSED, ""


def run_compiler_tests(compiler_exec_path: str, obj_dir: str, testinfo_dir: str,
                       coverage: bool) -> None:
    setup_coverage_dir(testinfo_dir)

    result_printer = ResultPrinter()

    for file in list_test_files():
        test_result, output, expected_output = run_test(file, compiler_exec_path)
        result_printer.print_result(file, test_result, output, expected_output)

    result_printer.print_summary()

    if coverage:
        prepare_coverage_report(obj_dir, testinfo_dir, result_printer.passed)


def run_all_llc_tests(compiler_exec_path: str) -> None:
    remove_files(".llc")

    result_printer = ResultPrinter()

    for file in list_test_files():
        test_result, output = run_test_llc(file, compiler_exec_path)
        result_printer.print_result(file, test_result, output)

    # Print number of tests that passed
    result_printer.print_summary()

    remove_files(".o")
    remove_files(".llc")
    remove_files(".llc.s")


def parse_args() -> Optional[str]:
    if len(sys.argv) == 1:
        return "debug"

    if len(sys.argv) > 2:
        return None

    if sys.argv[1] == "--coverage":
        return "gcov"

    if sys.argv[1] == "--profile":
        return "gprof"

    return None


def main() -> None:
    build_type = parse_args()

    if build_type is None:
        print("Invalid CLI args")
        return

    os.chdir("tests/")

    print("--=[Running ShnooTalk compiler tests]=--")
    run_compiler_tests(f"../bin/{build_type}/shtkc",
                       f"../obj/{build_type}/", "testinfo/",
                       build_type == "gcov")

    if build_type == "debug":
        print("--=[Running LLVM LLC tests]=--")
        run_all_llc_tests(f"../bin/{build_type}/shtkc")


if __name__ == "__main__":
    main()
