from typing import Callable

from tests_runner.utils.dir import list_test_files
from tests_runner.utils.result import ResultPrinter, TestResult


def batch_run(name: str, single_run_callback: Callable[[str], TestResult]) -> None:

    result_printer = ResultPrinter(name)

    for file in list_test_files():
        test_result = single_run_callback(file)
        result_printer.print_result(file, test_result)

    result_printer.print_summary()
