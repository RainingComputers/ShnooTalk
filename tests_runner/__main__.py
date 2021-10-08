import os

from tests_runner.config import BUILD_TYPE

from tests_runner import compiler_tests, llc_tests


def main() -> None:
    if BUILD_TYPE is None:
        print("Invalid CLI args")
        return

    os.chdir("tests/")

    compiler_tests.run()

    if BUILD_TYPE == "debug":
        llc_tests.run()


if __name__ == "__main__":
    main()
