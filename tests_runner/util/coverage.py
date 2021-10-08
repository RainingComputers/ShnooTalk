from typing import List

import os
from glob import glob

from tqdm.contrib.concurrent import process_map

from tests_runner.config import COVERAGE_INFO_DIR, OBJ_DIR


def shell(command: str) -> None:
    os.system(command + ' > /dev/null')


def setup_coverage_dir() -> None:
    shell(f"rm -rf {COVERAGE_INFO_DIR}")
    shell(f"mkdir -p {COVERAGE_INFO_DIR}")


def set_gmon_prefix(file_name: str) -> None:
    os.environ["GMON_OUT_PREFIX"] = file_name + ".gmon.out"


def generate_info_file(passed_test_file: str) -> None:
    shell(f"lcov -c  -b ../ -d {OBJ_DIR} -o {COVERAGE_INFO_DIR}/{passed_test_file}.info")


def prepare_coverage_report(passed_test_files: List[str]) -> None:
    print("Generating info files...")
    process_map(generate_info_file, passed_test_files, max_workers=8, unit='tests', ncols=80)

    os.chdir(COVERAGE_INFO_DIR)

    print("Preparing coverage report...")
    info_files = " ".join([f"-a {info_file}" for info_file in glob("*.info")])

    shell(f"lcov {info_files} -o total_unfiltered.info")
    shell("lcov --remove total_unfiltered.info '/usr/include/*' '/usr/lib/*' -o total.info")
    shell("genhtml total.info -o ./")
    shell("xdg-open index.html")

    os.chdir('..')
