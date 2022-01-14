import sys

from tests_runner.framework import tester

from tests_runner import parser_tests, ir_tests, compiler_tests, llc_tests, cli_tests


def main() -> int:
    parser_tests.register()
    ir_tests.register()
    compiler_tests.register()
    llc_tests.register()
    cli_tests.register()
    return tester.run()


if __name__ == "__main__":
    sys.exit(main())
