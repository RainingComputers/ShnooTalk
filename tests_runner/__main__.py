from typing import Tuple, List, Optional

import os
import glob
import subprocess
import sys

from tqdm import tqdm

from tests_runner.result_printer import TestResultType, ResultPrinter


def get_test_output(file_name: str) -> str:
    # Extract commented test case from beginning of the file
    test_output = ""

    with open(file_name) as test_program:
        while True:
            line = next(test_program)
            if line[0] != '#':
                break

            test_output += line[2:]

    return test_output


def compare_outputs(test_output: str, actual_output: Optional[str]) -> Tuple[int, str, str]:
    if test_output == actual_output or actual_output is None:
        return TestResultType.PASSED, '', test_output

    return TestResultType.FAILED, actual_output, test_output


def run_subprocess(command: List[str]) -> Tuple[bool, str, Optional[int]]:
    try:
        subp = subprocess.run(command,
                              stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=15)
        console_output = subp.stdout.decode('utf-8')
        console_err_output = subp.stderr.decode('utf-8')
    except subprocess.TimeoutExpired:
        return True, '', None

    return False, console_output+console_err_output, subp.returncode


def run_test(file_name: str, compiler_exec_path: str) -> Tuple[int, str, str]:
    test_output = get_test_output(file_name)

    # Remove all object files before running the test
    os.system('rm -f *.o')
    os.system('rm -f TestModules/*.o')
    os.system('rm -f ./test')

    # Set profile output file name
    os.environ['GMON_OUT_PREFIX'] = file_name + '.gmon.out'

    # Run the compiler
    compile_command = [compiler_exec_path, file_name, '-c']
    timedout, compiler_output, compiler_retcode = run_subprocess(compile_command)

    if timedout:
        return TestResultType.TIMEDOUT, '', test_output

    # If there was a compilation error, return the error message from the compiler
    if compiler_retcode != 0:
        return compare_outputs(test_output, compiler_output)

    # Link object file into an executable
    object_files = " ".join(glob.glob("*.o")+glob.glob("TestModules/*.o"))
    os.system(f'gcc {object_files} -o test -lm')

    # Run the executable and return the output from the executable
    timedout, exec_output, _ = run_subprocess(['./test'])

    if timedout:
        return TestResultType.TIMEDOUT, '', test_output

    # If the program/executable did not timeout, return program output
    return compare_outputs(test_output, exec_output)


def run_test_llc(file_name: str, compiler_exec_path: str) -> Tuple[int, str]:
    # Run the compiler
    compile_command = [compiler_exec_path, file_name, '-llvm']
    timedout, compiler_output, compiler_retcode = run_subprocess(compile_command)

    if timedout:
        return TestResultType.TIMEDOUT, ''

    if compiler_retcode != 0:
        return TestResultType.SKIPPED, ''

    # Run llc
    llc_file = file_name + ".llc"

    with open(llc_file, "w") as ll_f:
        ll_f.write(compiler_output)

    timedout, llc_output, llc_retcode = run_subprocess(["llc", llc_file])

    # Return test result
    if llc_retcode != 0:
        return TestResultType.FAILED, llc_output

    return TestResultType.PASSED, ''


def generate_info_files(obj_dir: str, testinfo_dir: str, passed_test_files: List[str]) -> None:
    print("Generating info files...")

    for file in tqdm(passed_test_files, ncols=80, unit="test"):
        os.system(
            f"lcov -c  -b ../ -d {obj_dir} -o {testinfo_dir}{file}.info > /dev/null")


def prepare_coverage_report(testinfo_dir: str) -> None:
    # Generate report
    print("Preparing coverage report...")

    add_files = " ".join(
        [f"-a {info_file}" for info_file in glob.glob(f"{testinfo_dir}*.info")])

    os.system(f"lcov {add_files} -o {testinfo_dir}total_unfiltered.info > /dev/null")

    os.system(f"lcov --remove {testinfo_dir}total_unfiltered.info \
        '/usr/include/*' '/usr/lib/*' -o {testinfo_dir}total.info > /dev/null")

    os.system(
        f"genhtml {testinfo_dir}total.info -o {testinfo_dir} > /dev/null")

    # Open report
    os.system(f"xdg-open {testinfo_dir}index.html")


def setup_test(testinfo_dir: str) -> None:
    # Clean
    os.system(f"rm -rf {testinfo_dir}")
    os.system(f"mkdir -p {testinfo_dir}")


def run_all_tests(compiler_exec_path: str, obj_dir: str, testinfo_dir: str, coverage: bool) -> None:
    setup_test(testinfo_dir)

    result_printer = ResultPrinter()

    for file in os.listdir():
        if not file.endswith(".shtk"):
            continue

        test_result, output, expected_output = run_test(file, compiler_exec_path)
        result_printer.print_result(file, test_result, output, expected_output)

    result_printer.print_summary()

    if coverage:
        generate_info_files(obj_dir, testinfo_dir, result_printer.passed)
        prepare_coverage_report(testinfo_dir)


def run_all_llc_tests(compiler_exec_path: str) -> None:
    os.system('rm -f ./*.llc')

    result_printer = ResultPrinter()

    for file in os.listdir():
        if not file.endswith(".shtk"):
            continue

        test_result, output = run_test_llc(file, compiler_exec_path)
        result_printer.print_result(file, test_result, output)

    # Print number of tests that passed
    result_printer.print_summary()

    os.system('rm -f *.o')
    os.system('rm -f *.llc.s')


def main() -> None:
    build_type = ''

    if len(sys.argv) == 1:
        build_type = "debug"
    elif len(sys.argv) > 2:
        print("Invalid CLI args")
        return
    elif sys.argv[1] == "--coverage":
        build_type = "gcov"
    elif sys.argv[1] == "--profile":
        build_type = "gprof"
    else:
        print("Invalid CLI args")
        return

    os.chdir("tests/")

    print("--=[Running ShnooTalk compiler tests]=--")
    run_all_tests(f"../bin/{build_type}/shtkc",
                  f"../obj/{build_type}/", "testinfo/",
                  build_type == "gcov")

    if build_type == "debug":
        print("--=[Running LLVM LLC tests]=--")
        run_all_llc_tests(f"../bin/{build_type}/shtkc")


if __name__ == "__main__":
    main()
