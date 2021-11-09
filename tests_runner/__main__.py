from tests_runner.utils.config import CLI_ARG, BUILD_TYPE_MAP
from tests_runner.utils.coverage import prepare_coverage_report

from tests_runner import compiler_tests, llc_tests, cli_tests, parser_tests, ir_tests


def print_usage() -> None:
    print("🙁 Invalid CLI ARGS, available option are:")
    print(f"    {' '.join(list(BUILD_TYPE_MAP.keys()))}")


def run_all_tests() -> None:
    parser_tests.run()
    compiler_tests.run()
    llc_tests.run()
    cli_tests.run()
    ir_tests.run()


def main() -> None:
    if CLI_ARG == "--test":
        run_all_tests()

    elif CLI_ARG == "--coverage":
        run_all_tests()
        prepare_coverage_report()

    else:
        print_usage()


if __name__ == "__main__":
    main()
