import sys

from tests_runner.framework import tester

from tests_runner import parser_tests, ir_tests, compiler_tests, llc_tests, cli_tests, stdlib_tests


def main() -> int:
    tester.generator("compiler/tests/parser", "expected/json", "json", "-json-ast")
    tester.generator("compiler/tests/parser", "expected/pretty", "txt", "-ast")
    tester.generator("compiler/tests/ir", "expected/json", "json", "-json-ir-all")
    tester.generator("compiler/tests/compiler", "expected/json", "json", "-json-icode-all", True)
    tester.generator("compiler/tests/compiler", "expected/pretty", "txt", "-icode-all", True)
    tester.exec_generator("compiler/tests/compiler", "expected/output")
    tester.exec_generator("stdlib/tests", "expected/output")
    tester.register([parser_tests, ir_tests, compiler_tests, llc_tests, cli_tests, stdlib_tests])

    return tester.run()


if __name__ == "__main__":
    sys.exit(main())
