import os

from tests_runner.utils.batch import batch_run
from tests_runner.utils.dir import remove_files
from tests_runner.utils.result import TestResult
from tests_runner.utils.validator import compile_phase, command_validator


def run_single(file_name: str) -> TestResult:
    llc_file = file_name + ".llc"

    return command_validator(
        compile_phase_result=compile_phase(
            file_name=file_name,
            compile_flag='-llvm',
            compiler_output_dump_file=llc_file,
            link_phase=False,
            skip_on_compile_error=True
        ),
        expected_on_compile_fail=None,
        command_on_compile_success=['llc', llc_file],
        expected_command_output=None,
    )


def run() -> None:
    os.chdir("tests/compiler")

    remove_files(".llc")

    batch_run("llc", run_single)

    remove_files(".o")
    remove_files(".llc")
    remove_files(".llc.s")

    os.chdir("../..")
