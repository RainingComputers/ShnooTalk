import os

from tests_runner.utils.config import CLI_ARG, BUILD_TYPE_MAP
from tests_runner.utils.generate import generate_test_cases
from tests_runner.utils.coverage import prepare_coverage_report

from tests_runner import compiler_tests, llc_tests, ast_ir_tests, cli_tests


def print_usage() -> None:
    print("🙁 Invalid CLI ARGS, available option are:")
    print(f"    {' '.join(list(BUILD_TYPE_MAP.keys()))}")


def print_gen_warning() -> None:
    print("🤖 Generating test cases...")
    print("🛑 Make sure the compiler has passed all core (compiler and llc) tests")
    print("   Review every change using git")


def run_all_tests() -> None:
    compiler_tests.run()
    ast_ir_tests.run()
    llc_tests.run()
    cli_tests.run()


def main() -> None:
    os.chdir("tests/")

    if CLI_ARG == "--gen-ast":
        print_gen_warning()
        generate_test_cases("ExpectedAST", "Pretty", "-ast", ".txt")
        generate_test_cases("ExpectedAST", "JSON", "-json-ast", ".json")

    elif CLI_ARG == "--gen-ir":
        print_gen_warning()
        generate_test_cases("ExpectedIR", "Pretty", "-ir", ".txt")
        generate_test_cases("ExpectedIR", "JSON", "-json-ir", ".json")

    elif CLI_ARG == "--test":
        run_all_tests()

    elif CLI_ARG == "--core":
        compiler_tests.run()
        llc_tests.run()

    elif CLI_ARG == "--coverage":
        run_all_tests()
        prepare_coverage_report()

    else:
        print_usage()


if __name__ == "__main__":
    main()
