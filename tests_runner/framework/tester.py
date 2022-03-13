from typing import List, Callable, Optional, Dict

import os

import types
from json.decoder import JSONDecodeError
from inspect import getmembers, isfunction

from tests_runner.framework.result import Result, ResultPrinter
from tests_runner.framework.fs import remove_files, list_test_files
from tests_runner.framework.fs import dirctx, dump_string_to_file, remove_if_exists
from tests_runner.framework.coverage import prepare_coverage_report
from tests_runner.framework.command import run_command
from tests_runner.framework.assertions import link_objects_into_bin

from tests_runner.framework.config import CLI_ARG, INVALID_CLI_ARGS
from tests_runner.framework.config import COMPILER_EXEC_PATH, COMPILER_BUILD_FAILED
from tests_runner.framework.config import COMPILER_NOT_FOUND
from tests_runner.framework.config import CLIArg


class Tester:
    def __init__(self) -> None:
        self._tests: List[Callable[[], None]] = []
        self._generators: List[Callable[[], None]] = []
        self._result_printers: Dict[str, ResultPrinter] = {}

    def upsert_printer(self, group: str, generator: bool = False) -> ResultPrinter:
        try:
            return self._result_printers[group]
        except KeyError:
            new_printer = ResultPrinter(group, generator)
            self._result_printers[group] = new_printer
            return new_printer

    @staticmethod
    def clean_files(ext_list: Optional[List[str]]) -> None:
        if ext_list is None:
            return

        for file_ext in ext_list:
            remove_files(file_ext)

    def batch_run(
        self, group: str, test_func: Callable[[str], Result], generator: bool = False
    ) -> None:

        result_printer = self.upsert_printer(group, generator)

        for file in list_test_files():
            try:
                test_result = test_func(file)
                result_printer.print_result(file, test_result)
            except FileNotFoundError as error:
                reason = f"file {error.filename} not found"
                result_printer.print_result(file, Result.invalid(reason))
            except JSONDecodeError as error:
                result_printer.print_result(file, Result.invalid(str(error)))

    def batch(
        self, path: str, clean: Optional[List[str]] = None
    ) -> Callable[[Callable[[str], Result]], Callable[[], None]]:

        def batch_decorator(test_func: Callable[[str], Result]) -> Callable[[], None]:

            @dirctx(path)
            def batch_run_wrapper() -> None:
                Tester.clean_files(clean)

                group = test_func.__name__
                self.batch_run(group, test_func)

                Tester.clean_files(clean)

            def register_test() -> None:
                self._tests.append(batch_run_wrapper)

            return register_test

        return batch_decorator

    def single(
        self, path: Optional[str] = None
    ) -> Callable[[Callable[[], Result]], Callable[[], None]]:

        def single_decorator(test_func: Callable[[], Result]) -> Callable[[], None]:

            @dirctx(path)
            def single_run_wrapper() -> None:
                name = test_func.__name__
                group = test_func.__module__
                result_printer = self.upsert_printer(group)
                test_result = test_func()
                result_printer.print_result(name, test_result)

            def register_test() -> None:
                self._tests.append(single_run_wrapper)

            return register_test

        return single_decorator

    def _run_tests_list(self) -> None:
        for test_func in self._tests:
            test_func()

    @staticmethod
    def register(test_modules: List[types.ModuleType]) -> None:
        for test_module in test_modules:
            test_module_functions = list(map(
                lambda member: member[1],
                getmembers(test_module, isfunction)
            ))

            register_functions = list(filter(
                lambda func: func.__module__ == __name__,
                test_module_functions
            ))

            for register_func in register_functions:
                register_func()

    def generator(
        self, path: str, output_path: str, output_ext: str, compiler_flag: str,
        ignore_error: bool = False
    ) -> None:
        def generator(test_file: str) -> Result:
            command = [COMPILER_EXEC_PATH, test_file, compiler_flag]
            timedout, output, exit_code = run_command(command)

            if timedout:
                return Result.timedout()

            if exit_code != 0:
                if ignore_error:
                    return Result.skipped()

                return Result.failed(output)

            dump_string_to_file(os.path.join(output_path, f"{test_file}.{output_ext}"), output)
            return Result.passed(output)

        @dirctx(path)
        def batch_generator() -> None:
            group = os.path.join(path, output_path)
            self.batch_run(group, generator, True)

        self._generators.append(batch_generator)

    def exec_generator(self, path: str, output_path: str) -> None:
        def generator(test_file: str) -> Result:
            remove_files(".o")
            remove_if_exists("./test_executable")
            output_file_path = os.path.join(output_path, f"{test_file}.txt")

            command = [COMPILER_EXEC_PATH, test_file, "-c"]
            timedout, compiler_output, exit_code = run_command(command)

            if timedout:
                return Result.timedout()

            if exit_code != 0:
                dump_string_to_file(output_file_path, compiler_output)
                return Result.passed(compiler_output)

            link_objects_into_bin()

            command = ["./test_executable"]
            timedout, exec_output, _ = run_command(command)

            if timedout:
                return Result.timedout()

            dump_string_to_file(output_file_path, exec_output)
            return Result.passed(exec_output)

        @dirctx(path)
        def batch_generator() -> None:
            group = os.path.join(path, output_path)
            self.batch_run(group, generator, True)

        self._generators.append(batch_generator)

    def _run_generators(self) -> None:
        for gen_func in self._generators:
            gen_func()

    def run(self) -> int:
        if INVALID_CLI_ARGS or COMPILER_NOT_FOUND or COMPILER_BUILD_FAILED:
            return -1

        if CLI_ARG == CLIArg.GEN:
            self._run_generators()
            return ResultPrinter.exit_code

        self._run_tests_list()

        if CLI_ARG == CLIArg.COVERAGE:
            prepare_coverage_report()

        ResultPrinter.print_summary()
        print("🏁 Done.")

        return ResultPrinter.exit_code


tester = Tester()
