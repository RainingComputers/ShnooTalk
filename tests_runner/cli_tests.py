import os
from typing import List

from tests_runner.framework import run_command
from tests_runner.framework import TestResult, ResultPrinter

from tests_runner.framework import COMPILER_EXEC_PATH, VERSION_FILE
from tests_runner.framework import string_from_file

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
    cmd = [COMPILER_EXEC_PATH, "-version"]

    timedout, output, exit_code = run_command(cmd)

    if exit_code != 0 or timedout:
        return TestResult.failed(output)

    if string_from_file(VERSION_FILE) == output:
        return TestResult.failed(output)

    return TestResult.passed(output)


def run_invalid_args(args: List[str]) -> TestResult:
    cmd = [COMPILER_EXEC_PATH] + args

    timedout, output, exit_code = run_command(cmd)

    if exit_code == 0 or timedout:
        return TestResult.failed(output)

    if output != USAGE_HELP:
        return TestResult.failed(output, USAGE_HELP)

    return TestResult.passed(output)


def run_file_no_exists() -> TestResult:
    timedout, output, exit_code = run_command([COMPILER_EXEC_PATH, "NoExist.shtk", "-c"])

    if exit_code == 0 or timedout:
        return TestResult.failed(output)

    if output != FILE_IO_ERROR:
        return TestResult.failed(output, FILE_IO_ERROR)

    return TestResult.passed(output)


def run() -> None:
    os.chdir("tests/compiler")

    printer = ResultPrinter('CLI args')

    printer.print_result('No args', run_invalid_args([]))

    printer.print_result(
        'Invalid args',
        run_invalid_args(["TestModules/Math.shtk", "-invalid"])
    )

    printer.print_result(
        'Too many args',
        run_invalid_args(["TestModules/Math.shtk", "-invalid", "-too-many"])
    )

    printer.print_result('File not found', run_file_no_exists())

    printer.print_result('-version', run_version_test())

    os.chdir("../..")
