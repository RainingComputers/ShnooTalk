from typing import Callable

from tests_runner.util.dir import list_test_files
from tests_runner.util.result import ResultPrinter, TestResult
from tests_runner.util.coverage import prepare_coverage_report


def batch_run(name: str, single_run_callback: Callable[[str], TestResult],
              coverage: bool = False) -> None:

    result_printer = ResultPrinter(name)

    for file in list_test_files():
        test_result = single_run_callback(file)
        result_printer.print_result(file, test_result)

    result_printer.print_summary()

    if coverage:
        prepare_coverage_report(result_printer.passed)
