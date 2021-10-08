from tests_runner.config import COMPILER_EXEC_PATH

from tests_runner.util.run_subprocess import run_subprocess
from tests_runner.util.test_dir import list_test_files
from tests_runner.util.test_dir import remove_files
from tests_runner.util.result import TestResult, ResultPrinter


def run_single(file_name: str, compiler_exec_path: str) -> TestResult:
    # Run the compiler
    compile_command = [compiler_exec_path, file_name, "-llvm"]
    timedout, compiler_output, compiler_retcode = run_subprocess(compile_command)

    if timedout:
        return TestResult.timedout()

    if compiler_retcode != 0:
        return TestResult.skipped()

    # Run llc
    llc_file = file_name + ".llc"

    with open(llc_file, "w") as ll_f:
        ll_f.write(compiler_output)

    timedout, llc_output, llc_retcode = run_subprocess(["llc", llc_file])

    # Return test result
    if llc_retcode != 0:
        return TestResult.failed(llc_output)

    return TestResult.passed()


def run() -> None:
    remove_files(".llc")

    result_printer = ResultPrinter("llc")

    for file in list_test_files():
        test_result = run_single(file, COMPILER_EXEC_PATH)
        result_printer.print_result(file, test_result)

    # Print number of tests that passed
    result_printer.print_summary()

    remove_files(".o")
    remove_files(".llc")
    remove_files(".llc.s")
