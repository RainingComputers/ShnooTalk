import sys

from tests_runner.framework import CLI_ARG, CLI_ARG_OPTIONS
from tests_runner.framework import ResultPrinter
from tests_runner.framework import prepare_coverage_report

from tests_runner import compiler_tests, llc_tests, cli_tests, parser_tests, ir_tests


def print_usage() -> None:
    print("🙁 Invalid CLI ARGS, available option are:")
    print(f"    {CLI_ARG_OPTIONS}")


def run_all_tests() -> None:
    parser_tests.run()
    ir_tests.run()
    compiler_tests.run()
    llc_tests.run()
    cli_tests.run()


def print_done() -> None:
    print("🏁 Done.")


def main() -> int:
    if CLI_ARG == "--test":
        run_all_tests()
        print_done()
        return ResultPrinter.exit_code

    if CLI_ARG == "--coverage":
        run_all_tests()
        prepare_coverage_report()
        print_done()
        return ResultPrinter.exit_code

    print_usage()
    return -1


if __name__ == "__main__":
    sys.exit(main())
