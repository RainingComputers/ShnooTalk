from typing import List

import os
import glob

from tqdm import tqdm

from tests_runner.config import COVERAGE_INFO_DIR, OBJ_DIR


def setup_coverage_dir() -> None:
    os.system(f"rm -rf {COVERAGE_INFO_DIR}")
    os.system(f"mkdir -p {COVERAGE_INFO_DIR}")


def generate_info_files(passed_test_files: List[str]) -> None:
    print("Generating info files...")

    for file in tqdm(passed_test_files, ncols=80, unit="test"):
        os.system(f"lcov -c  -b ../ -d {OBJ_DIR} -o {COVERAGE_INFO_DIR}{file}.info > /dev/null")


def prepare_coverage_report(passed_test_files: List[str]) -> None:
    generate_info_files(passed_test_files)

    # Generate report
    print("Preparing coverage report...")

    info_files = " ".join(
        [f"-a {info_file}" for info_file in glob.glob(f"{COVERAGE_INFO_DIR}*.info")])

    os.system(f"lcov {info_files} -o {COVERAGE_INFO_DIR}total_unfiltered.info > /dev/null")

    os.system(f"lcov --remove {COVERAGE_INFO_DIR}total_unfiltered.info \
        '/usr/include/*' '/usr/lib/*' -o {COVERAGE_INFO_DIR}total.info > /dev/null")

    os.system(f"genhtml {COVERAGE_INFO_DIR}total.info -o {COVERAGE_INFO_DIR} > /dev/null")

    # Open report
    os.system(f"xdg-open {COVERAGE_INFO_DIR}index.html")
