from tests_runner.framework import TestResult

from tests_runner.framework import COMPILER_VERSION
from tests_runner.framework import simple_output_assert

from tests_runner.framework import tester

TEST_GROUP = 'CLI args'

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


@tester.single(TEST_GROUP, '-version')
def run_version_test() -> TestResult:
    return simple_output_assert(["-version"], COMPILER_VERSION, False)


@tester.single(TEST_GROUP, 'No args')
def run_no_args() -> TestResult:
    return simple_output_assert([], USAGE_HELP, True)


@tester.single(TEST_GROUP, 'Invalid args', "tests/compiler")
def run_invalid_args() -> TestResult:
    return simple_output_assert(["TestModules/Math.shtk", "-invalid"], USAGE_HELP, True)


@tester.single(TEST_GROUP, 'Too many args')
def run_too_many_args() -> TestResult:
    return simple_output_assert(
        ["TestModules/Math.shtk", "-invalid", "-too-many"],
        USAGE_HELP, True
    )


@tester.single(TEST_GROUP, '-version', "tests/compiler")
def run_file_no_exists() -> TestResult:
    return simple_output_assert(["NoExist.shtk", "-c"], FILE_IO_ERROR, True)


def register() -> None:
    run_version_test()
    run_no_args()
    run_invalid_args()
    run_too_many_args()
    run_file_no_exists()
