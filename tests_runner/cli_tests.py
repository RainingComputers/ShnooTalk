from tests_runner.framework import TestResult

from tests_runner.framework import COMPILER_VERSION
from tests_runner.framework import simple_output_assert

from tests_runner.framework import tester

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


@tester.single()
def version() -> TestResult:
    return simple_output_assert(["-version"], COMPILER_VERSION, False)


@tester.single()
def no_args() -> TestResult:
    return simple_output_assert([], USAGE_HELP, True)


@tester.single("tests/compiler")
def invalid_args() -> TestResult:
    return simple_output_assert(["TestModules/Math.shtk", "-invalid"], USAGE_HELP, True)


@tester.single()
def too_many_args() -> TestResult:
    return simple_output_assert(
        ["TestModules/Math.shtk", "-invalid", "-too-many"],
        USAGE_HELP, True
    )


@tester.single("tests/compiler")
def file_does_not_exists() -> TestResult:
    return simple_output_assert(["NoExist.shtk", "-c"], FILE_IO_ERROR, True)


def register() -> None:
    version()
    no_args()
    invalid_args()
    too_many_args()
    file_does_not_exists()
