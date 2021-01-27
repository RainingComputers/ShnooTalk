import os
import glob
import subprocess

from difflib import ndiff
from enum import Enum


class TestResult(Enum):
    PASSED = 0
    FAILED = 1
    TIMEDOUT = 2


def run_test(file_name, exec_path):
    test_output = ""

    with open(file_name) as test_program:
        # Get the test output from comments in first few lines
        while True:
            line = next(test_program)
            if(line[0] != '#'):
                break
            else:
                test_output += line[2:]

    # Run the program and get the actual output of the program
    try:
        actual_output = subprocess.run([exec_path, file_name],
                                       stdout=subprocess.PIPE, timeout=15).stdout.decode('utf-8')
    except subprocess.TimeoutExpired as e:
        return TestResult.TIMEDOUT, None, test_output

    # Compare them
    if (test_output == actual_output):
        return TestResult.PASSED, actual_output, test_output
    else:
        return TestResult.FAILED, actual_output, test_output


def run_all_tests(exec_path, obj_dir, src_dir, testinfo_dir):
    # Clean
    os.system(f"rm -rf {testinfo_dir}")
    os.system(f"mkdir -p {testinfo_dir}")

    # Copy source files
    os.system(f"cp {src_dir}*.cpp {obj_dir}")

    # Run each test in a subprocess
    failed = []
    passed = []

    for file in os.listdir():
        if file.endswith("_test.uhll"):
            res, act_output, test_output = run_test(file, exec_path)

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

                diff = ndiff(act_output.splitlines(keepends=True),
                             test_output.splitlines(keepends=True))

                print("[Diff]")
                print(''.join(diff))

                failed.append(file)
            elif(res == TestResult.TIMEDOUT):
                print(" 🕒", file, "timedout")

    # Generate report
    print("Preparing coverage report...")
    add_files = " ".join(
        [f"-a {info_file}" for info_file in glob.glob(f"{testinfo_dir}*.info")])
    os.system(f"lcov {add_files} -o {testinfo_dir}total.info > /dev/null")
    os.system(
        f"genhtml {testinfo_dir}total.info -o {testinfo_dir} > /dev/null")

    # Open report
    os.system(f"xdg-open {testinfo_dir}index.html")


if __name__ == "__main__":
    run_all_tests("../bin/debug/uhll", "../obj/debug/", "../src/", "testinfo/")
