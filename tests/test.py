import os
import glob
import subprocess

from difflib import ndiff
from enum import Enum


class TestResult(Enum):
    PASSED = 0
    FAILED = 1
    TIMEDOUT = 2


def get_test_output(file_name):
    # Extract commented test case from beginning of the file
    test_output = ""

    with open(file_name) as test_program:
        while True:
            line = next(test_program)
            if(line[0] != '#'):
                break

            test_output += line[2:]

    return test_output


def compare_outputs(test_output, actual_output):
    if (test_output == actual_output):
        return TestResult.PASSED, actual_output, test_output

    return TestResult.FAILED, actual_output, test_output


def run_subprocess(command):
    try:
        subp = subprocess.run(command,
                              stdout=subprocess.PIPE, timeout=15)
        console_output = subp.stdout.decode('utf-8')
    except subprocess.TimeoutExpired:
        return True, None, None

    return False, console_output, subp.returncode


def run_test(file_name, compiler_exec_path):
    test_output = get_test_output(file_name)
    exec_output = ""
    compiler_output = ""

    # Remove all object files before running the test
    os.system('rm -f *.o')
    os.system('rm -f ./test')

    # Run the compiler
    compile_command = [compiler_exec_path, file_name, '-c']
    timedout, compiler_output, compiler_retcode = run_subprocess(
        compile_command)

    if(timedout):
        return TestResult.TIMEDOUT, None, test_output

    # If there was a compilation error, return the error message from the compiler
    if(compiler_retcode != 0):
        return compare_outputs(test_output, compiler_output)

    # Link pbject file int an execuatable
    os.system('clang *.o -o test')

    # Run the executable and return the output from the executable
    timedout, exec_output, _ = run_subprocess(['./test'])
    
    if(timedout):
        return TestResult.TIMEDOUT, None, test_output

    # If the program/executable did not timeout, return program output 
    return compare_outputs(test_output, exec_output)


def prepare_coverage_report(testinfo_dir):
    # Generate report
    print("Preparing coverage report...")
    add_files = " ".join(
        [f"-a {info_file}" for info_file in glob.glob(f"{testinfo_dir}*.info")])
    os.system(f"lcov {add_files} -o {testinfo_dir}total.info > /dev/null")
    os.system(
        f"genhtml {testinfo_dir}total.info -o {testinfo_dir} > /dev/null")

    # Open report
    os.system(f"xdg-open {testinfo_dir}index.html")


def setup_test(obj_dir, src_dir, testinfo_dir):
    # Clean
    os.system(f"rm -rf {testinfo_dir}")
    os.system(f"mkdir -p {testinfo_dir}")

    # Copy source files
    os.system(f"cp {src_dir}*.cpp {obj_dir}")


def run_all_tests(compiler_exec_path, obj_dir, src_dir, testinfo_dir):
    setup_test(obj_dir, src_dir, testinfo_dir)

    # Run each test in a subprocess
    failed = []
    passed = []

    for file in os.listdir():
        if not file.endswith("_test.uhll"):
            continue

        res, act_output, test_output = run_test(file, compiler_exec_path)

        if(res == TestResult.PASSED):
            print(" 👌", file, "passed")
            passed.append(file)
            # Process .gcda and .gcno files with lcov
            os.system(
                f"lcov -c -d {obj_dir} -o {testinfo_dir}{file}.info > /dev/null")
        elif(res == TestResult.FAILED):
            print(" ❌", file, "failed\n")
            print("[Program output]")
            print(act_output)
            print("[Defined test output]")
            print(test_output)
            failed.append(file)
        elif(res == TestResult.TIMEDOUT):
            print(" 🕒", file, "timedout")

    # Print number of tests that passed
    print(f"{len(failed)} tests failed.")
    print(f"{len(passed)} tests passed.")

    # Use lcov and open report in browser
    prepare_coverage_report(testinfo_dir)


if __name__ == "__main__":
    run_all_tests("../bin/debug/uhll", "../obj/debug/", "../src/", "testinfo/")
