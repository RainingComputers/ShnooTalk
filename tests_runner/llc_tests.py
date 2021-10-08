from tests_runner.util.dir import list_test_files
from tests_runner.util.dir import remove_files
from tests_runner.util.result import TestResult, ResultPrinter
from tests_runner.util.phase import phase_executer


def run_single(file_name: str) -> TestResult:
    llc_file = file_name + ".llc"

    return phase_executer(
        file_name=file_name,
        compile_flag='-llvm',
        compiler_output_dump_file=llc_file,
        command=['llc', llc_file],
        link_phase=False,
        skip_on_compile_error=True
    )


def run() -> None:
    remove_files(".llc")

    result_printer = ResultPrinter("llc")

    for file in list_test_files():
        test_result = run_single(file)
        result_printer.print_result(file, test_result)

    # Print number of tests that passed
    result_printer.print_summary()

    remove_files(".o")
    remove_files(".llc")
    remove_files(".llc.s")
