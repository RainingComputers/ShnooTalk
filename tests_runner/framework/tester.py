from typing import List, Callable, Optional, Dict

import os
from json.decoder import JSONDecodeError

from tests_runner.framework.result import TestResult, ResultPrinter
from tests_runner.framework.fs import remove_files
from tests_runner.framework.fs import list_test_files
from tests_runner.framework.coverage import prepare_coverage_report

from tests_runner.framework.config import CLI_ARG, CLI_ARG_OPTIONS, COMPILER_EXEC_PATH


class Tester:
    def __init__(self) -> None:
        self._tests: List[Callable[[], None]] = []
        self._result_printers: Dict[str, ResultPrinter] = {}

    def upsert_printer(self, group: str) -> ResultPrinter:
        try:
            return self._result_printers[group]
        except KeyError:
            new_printer = ResultPrinter(group)
            self._result_printers[group] = new_printer
            return new_printer

    @staticmethod
    def clean_files(ext_list: Optional[List[str]]) -> None:
        if ext_list is None:
            return

        for file_ext in ext_list:
            remove_files(file_ext)

    def batch_run(self, group: str, test_func: Callable[[str], TestResult]) -> None:

        result_printer = self.upsert_printer(group)

        for file in list_test_files():
            try:
                test_result = test_func(file)
                result_printer.print_result(file, test_result)
            except FileNotFoundError as error:
                reason = f"file {error.filename} not found"
                result_printer.print_result(file, TestResult.invalid(reason))
            except JSONDecodeError as error:
                result_printer.print_result(file, TestResult.invalid(str(error)))

    def batch(
        self, path: str, clean: Optional[List[str]] = None
    ) -> Callable[[Callable[[str], TestResult]], Callable[[], None]]:

        def batch_decorator(test_func: Callable[[str], TestResult]) -> Callable[[], None]:

            def batch_run_wrapper() -> None:
                saved_path = os.getcwd()
                os.chdir(path)
                Tester.clean_files(clean)

                group = test_func.__name__
                self.batch_run(group, test_func)

                Tester.clean_files(clean)
                os.chdir(saved_path)

            def register_test() -> None:
                self._tests.append(batch_run_wrapper)

            return register_test

        return batch_decorator

    def single(
        self, group: str, path: Optional[str] = None
    ) -> Callable[[Callable[[], TestResult]], Callable[[], None]]:

        def single_decorator(test_func: Callable[[], TestResult]) -> Callable[[], None]:

            def single_run_wrapper() -> None:
                saved_path = os.getcwd()

                if path is not None:
                    os.chdir(path)

                result_printer = self.upsert_printer(group)
                test_result = test_func()
                name = test_func.__name__
                result_printer.print_result(name, test_result)

                if path is not None:
                    os.chdir(saved_path)

            def register_test() -> None:
                self._tests.append(single_run_wrapper)

            return register_test

        return single_decorator

    def _run_tests_list(self) -> None:
        for test_func in self._tests:
            test_func()

    @staticmethod
    def _print_done() -> None:
        print("🏁 Done.")

    @staticmethod
    def _print_usage() -> None:
        print("🙁 Invalid CLI ARGS, available option are:")
        print(f"    {CLI_ARG_OPTIONS}")

    @staticmethod
    def _print_compiler_not_found() -> None:
        print(f"🙁 compiler not found at {COMPILER_EXEC_PATH}")

    def run(self) -> int:
        if CLI_ARG is None:
            Tester._print_usage()
            return -1

        if not os.path.exists(COMPILER_EXEC_PATH):
            self._print_compiler_not_found()
            return -1

        self._run_tests_list()

        if CLI_ARG == "--coverage":
            prepare_coverage_report()

        ResultPrinter.print_summary()
        Tester._print_done()

        return ResultPrinter.exit_code


tester = Tester()
