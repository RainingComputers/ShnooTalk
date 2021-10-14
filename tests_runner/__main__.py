import os

from tests_runner.utils.config import CLI_ARG
from tests_runner.utils.generate import generate_test_cases

from tests_runner import compiler_tests, llc_tests, ast_ir_tests


def main() -> None:
    os.chdir("tests/")

    if CLI_ARG is None:
        print("Invalid CLI args")
        return

    if CLI_ARG == "ast_gen":
        generate_test_cases("ExpectedAST", "Pretty", "-ast", ".txt")
        return

    if CLI_ARG == "ast_json_gen":
        generate_test_cases("ExpectedAST", "JSON", "-json-ast", ".json")
        return

    if CLI_ARG == "ir_gen":
        generate_test_cases("ExpectedIR", "Pretty", "-ir", ".txt")
        return

    if CLI_ARG == "ir_json_gen":
        generate_test_cases("ExpectedIR", "JSON", "-json-ir", ".json")
        return

    compiler_tests.run()

    if CLI_ARG == "debug":
        llc_tests.run()
        ast_ir_tests.run()


if __name__ == "__main__":
    main()
