import os

from tests_runner.framework import TestResult, ResultPrinter

from tests_runner.framework import COMPILER_VERSION
from tests_runner.framework import simple_output_assert

USAGE_HELP = '''USAGE: shtkc FILE OPTION

Available options:
    -c               Compile program
    -ast             Print parse tree
    -ir              Print ShnooTalk IR
    -icode           Print ShnooTalk IR, but only the icode
    -ir-all          Print ShnooTalk IR recursively for all modules
    -llvm            Print LLVM IR
    -json-ast        Print parse tree in JSON
    -json-ir         Print ShnooTalk IR in JSON
    -json-ir-all     Print ShnooTalk IR recursively for all modules in json
    -json-icode      Print ShnooTalk IR in JSON, but only the icode

Use shtkc -version for compiler version
'''

FILE_IO_ERROR = "File I/O error\n"


def run_version_test() -> TestResult:
    return simple_output_assert(["-version"], COMPILER_VERSION, False)


def run_no_args() -> TestResult:
    return simple_output_assert([], USAGE_HELP, True)


def run_invalid_args() -> TestResult:
    return simple_output_assert(["TestModules/Math.shtk", "-invalid"], USAGE_HELP, True)


def run_too_many_args() -> TestResult:
    return simple_output_assert(
        ["TestModules/Math.shtk", "-invalid", "-too-many"],
        USAGE_HELP, True
    )


def run_file_no_exists() -> TestResult:
    return simple_output_assert(["NoExist.shtk", "-c"], FILE_IO_ERROR, True)


def run() -> None:
    os.chdir("tests/compiler")

    printer = ResultPrinter('CLI args')

    printer.print_result('No args', run_no_args())

    printer.print_result('Invalid args', run_invalid_args())

    printer.print_result('Too many args', run_too_many_args())

    printer.print_result('File not found', run_file_no_exists())

    printer.print_result('-version', run_version_test())

    os.chdir("../..")
