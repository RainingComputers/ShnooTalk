import os
import glob
import subprocess
import sys
from difflib import ndiff
from tqdm import tqdm


class TestResult:
    PASSED = 0
    FAILED = 1
    TIMEDOUT = 2
    SKIPPED = 3


def get_build_type():
    if(len(sys.argv) == 1):
        return "debug"
    
    if(sys.argv[1] == "--coverage"):
        return "gcov"
    
    if(sys.argv[1] == "--profile"):
        return "gprof"


def print_diff(act_output, test_output):
    diff = ndiff(act_output.splitlines(keepends=True),
                 test_output.splitlines(keepends=True))

    print(''.join(diff))


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
                              stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=15)
        console_output = subp.stdout.decode('utf-8')
        console_err_output = subp.stderr.decode('utf-8')
    except subprocess.TimeoutExpired:
        return True, None, None

    return False, console_output+console_err_output, subp.returncode


def run_test(file_name, compiler_exec_path):
    test_output = get_test_output(file_name)
    exec_output = ""
    compiler_output = ""

    # Remove all object files before running the test
    os.system('rm -f *.o')
    os.system('rm -f TestModules/*.o')
    os.system('rm -f ./test')

    # Set profile output file name
    os.environ['GMON_OUT_PREFIX'] = file_name + '.gmon.out'

    # Run the compiler
    compile_command = [compiler_exec_path, file_name, '-c']
    timedout, compiler_output, compiler_retcode = run_subprocess(
        compile_command)

    if(timedout):
        return TestResult.TIMEDOUT, None, test_output

    # If there was a compilation error, return the error message from the compiler
    if(compiler_retcode != 0):
        return compare_outputs(test_output, compiler_output)

    # Link object file into an executable
    object_files = " ".join(glob.glob("*.o")+glob.glob("TestModules/*.o"))
    os.system(f'gcc {object_files} -o test -lm')

    # Run the executable and return the output from the executable
    timedout, exec_output, _ = run_subprocess(['./test'])

    if(timedout):
        return TestResult.TIMEDOUT, None, test_output

    # If the program/executable did not timeout, return program output
    return compare_outputs(test_output, exec_output)


def run_test_llc(file_name, compiler_exec_path):    
    # Run the compiler
    compile_command = [compiler_exec_path, file_name, '-llvm']
    timedout, compiler_output, compiler_retcode = run_subprocess(
        compile_command)

    if(timedout):
        return TestResult.TIMEDOUT, None

    if(compiler_retcode != 0):
        return TestResult.SKIPPED, None

    # Run llc
    llc_file = file_name + ".llc"

    with open(llc_file, "w") as f:
        f.write(compiler_output)

    timedout, llc_output, llc_retcode = run_subprocess(
        ["llc", llc_file])

    # Return test result
    if(llc_retcode != 0):
        return TestResult.FAILED, llc_output

    return TestResult.PASSED, None


def generate_info_files(obj_dir, testinfo_dir, passed_test_files):
    print("Generating info files...")

    for file in tqdm(passed_test_files, ncols=80, unit="test"):
        os.system(
            f"lcov -c  -b ../ -d {obj_dir} -o {testinfo_dir}{file}.info > /dev/null")


def prepare_coverage_report(testinfo_dir):
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


def setup_test(testinfo_dir):
    # Clean
    os.system(f"rm -rf {testinfo_dir}")
    os.system(f"mkdir -p {testinfo_dir}")


def run_all_tests(compiler_exec_path, obj_dir, testinfo_dir):
    setup_test(testinfo_dir)

    # Run each test in a subprocess
    failed = []
    passed = []

    for file in os.listdir():
        if not file.endswith(".shtk"):
            continue

        res, act_output, test_output = run_test(file, compiler_exec_path)

        if(res == TestResult.PASSED):
            print(" 👌", file, "passed")
            passed.append(file)
        elif(res == TestResult.FAILED):
            print(" ❌", file, "failed\n")
            print("[Program output]")
            print(act_output)
            print("[Defined test output]")
            print(test_output)
            print("[Diff]")
            print_diff(act_output, test_output)
            failed.append(file)
        elif(res == TestResult.TIMEDOUT):
            print(" 🕒", file, "timedout")

    # Print number of tests that passed
    print(f"{len(failed)} tests failed.")
    print(f"{len(passed)} tests passed.")

    # Use lcov and open report in browser
    if(get_build_type() == 'gcov'):
        generate_info_files(obj_dir, testinfo_dir, passed)
        prepare_coverage_report(testinfo_dir)


def run_all_llc_tests(compiler_exec_path):
    os.system('rm -f ./*.llc')

    failed = []
    passed = []

    for file in os.listdir():
        if not file.endswith(".shtk"):
            continue

        res, llc_output = run_test_llc(file, compiler_exec_path)

        if(res == TestResult.PASSED):
            print(" 👌", file, "passed")
            passed.append(file)
        elif(res == TestResult.FAILED):
            print(" ❌", file, "failed\n")
            print("[LLC output]")
            print(llc_output)
            failed.append(file)
        elif(res == TestResult.TIMEDOUT):
            print(" 🕒", file, "timedout")

    # Print number of tests that passed
    print(f"{len(failed)} tests failed.")
    print(f"{len(passed)} tests passed.")

    os.system('rm -f *.o')
    os.system('rm -f *.llc.s')


if __name__ == "__main__":
    build_type = get_build_type()

    os.chdir("tests/")

    print("--=[Running ShnooTalk compiler tests]=--")
    run_all_tests(f"../bin/{build_type}/shtkc",
                  f"../obj/{build_type}/", "testinfo/")

    if (build_type == "debug"):
        print("--=[Running LLVM LLC tests]=--")
        run_all_llc_tests(f"../bin/{build_type}/shtkc")
