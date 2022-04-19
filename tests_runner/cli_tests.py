from tests_runner.framework import Result

from tests_runner.framework import COMPILER_VERSION
from tests_runner.framework import simple_output_assert

from tests_runner.framework import tester

USAGE_HELP = '''USAGE: shtkc FILE OPTION

Available options:
    -c               Create debug executable
    -release         Create release executable
    -ast             Print parse tree
    -ir              Print ShnooTalk IR
    -icode           Print ShnooTalk IR, but only the icode
    -ir-all          Print ShnooTalk IR recursively for all modules
    -llvm            Print LLVM IR
    -llvm-release    Print LLVM IR after optimization
    -json-ast        Print parse tree in JSON
    -json-ir         Print ShnooTalk IR in JSON
    -json-ir-all     Print ShnooTalk IR recursively for all modules in json
    -json-icode      Print ShnooTalk IR in JSON, but only the icode

Use shtkc -version for compiler version
'''

FILE_IO_ERROR = "File I/O error\n"


@tester.single()
def version() -> Result:
    return simple_output_assert(["-version"], COMPILER_VERSION, False)


@tester.single()
def no_args() -> Result:
    return simple_output_assert([], USAGE_HELP, True)


@tester.single("tests/compiler")
def invalid_args() -> Result:
    return simple_output_assert(["TestModules/Math.shtk", "-invalid"], USAGE_HELP, True)


@tester.single()
def too_many_args() -> Result:
    return simple_output_assert(
        ["TestModules/Math.shtk", "-invalid", "-too-many"],
        USAGE_HELP, True
    )


@tester.single("tests/compiler")
def file_does_not_exists() -> Result:
    return simple_output_assert(["NoExist.shtk", "-c"], FILE_IO_ERROR, True)
