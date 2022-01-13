from typing import Callable

from json.decoder import JSONDecodeError

from tests_runner.framework.fs import list_test_files
from tests_runner.framework.result import ResultPrinter, TestResult


def batch_run(name: str, single_run_callback: Callable[[str], TestResult]) -> None:

    result_printer = ResultPrinter(name)

    for file in list_test_files():
        try:
            test_result = single_run_callback(file)
            result_printer.print_result(file, test_result)
        except FileNotFoundError as error:
            reason = f"file {error.filename} not found"
            result_printer.print_result(file, TestResult.invalid(reason))
        except JSONDecodeError as error:
            result_printer.print_result(file, TestResult.invalid(str(error)))

    result_printer.print_summary()
