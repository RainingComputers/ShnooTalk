from tests_runner.config import BUILD_TYPE

from tests_runner.util.dir import list_test_files
from tests_runner.util.result import TestResult, ResultPrinter
from tests_runner.util.coverage import prepare_coverage_report
from tests_runner.util.validator import compile_phase, validate


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


def run_single(file_name: str) -> TestResult:
    expected_output = get_expected_output(file_name)

    return validate(
        compile_phase_result=compile_phase(
            file_name=file_name,
            compile_flag="-c",
            compiler_output_dump_file=None,
            link_phase=True,
            skip_on_compile_error=False,
        ),
        expected_on_compile_fail=expected_output,
        command_on_compile_success=["./test_executable"],
        expected_command_output=expected_output
    )


def run() -> None:
    result_printer = ResultPrinter("ShnooTalk compiler")

    for file in list_test_files():
        test_result = run_single(file)
        result_printer.print_result(file, test_result)

    result_printer.print_summary()

    if BUILD_TYPE == "gcov":
        prepare_coverage_report(result_printer.passed)
