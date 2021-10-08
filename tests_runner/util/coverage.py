from typing import List

import os
import glob

from tqdm import tqdm


def setup_coverage_dir(testinfo_dir: str) -> None:
    os.system(f"rm -rf {testinfo_dir}")
    os.system(f"mkdir -p {testinfo_dir}")


def generate_info_files(obj_dir: str, testinfo_dir: str, passed_test_files: List[str]) -> None:
    print("Generating info files...")

    for file in tqdm(passed_test_files, ncols=80, unit="test"):
        os.system(f"lcov -c  -b ../ -d {obj_dir} -o {testinfo_dir}{file}.info > /dev/null")


def prepare_coverage_report(obj_dir: str, testinfo_dir: str, passed_test_files: List[str]) -> None:
    generate_info_files(obj_dir, testinfo_dir, passed_test_files)

    # Generate report
    print("Preparing coverage report...")

    add_files = " ".join(
        [f"-a {info_file}" for info_file in glob.glob(f"{testinfo_dir}*.info")])

    os.system(f"lcov {add_files} -o {testinfo_dir}total_unfiltered.info > /dev/null")

    os.system(f"lcov --remove {testinfo_dir}total_unfiltered.info \
        '/usr/include/*' '/usr/lib/*' -o {testinfo_dir}total.info > /dev/null")

    os.system(f"genhtml {testinfo_dir}total.info -o {testinfo_dir} > /dev/null")

    # Open report
    os.system(f"xdg-open {testinfo_dir}index.html")
