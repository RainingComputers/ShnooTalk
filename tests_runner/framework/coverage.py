import os

from tests_runner.framework.config import COVERAGE_REPORT_DIR, OBJ_DIR, OPEN_BIN, LCOV_EXCLUDE


def shell(command: str) -> None:
    os.system(command + ' > /dev/null')


def set_gmon_prefix(file_name: str) -> None:
    os.environ["GMON_OUT_PREFIX"] = file_name + ".gmon.out"


def setup_coverage_dir() -> None:
    shell(f"rm -rf {COVERAGE_REPORT_DIR}")
    shell(f"mkdir -p {COVERAGE_REPORT_DIR}")


def prepare_coverage_report() -> None:
    os.chdir("tests")
    print("🤖 Preparing coverage report...")
    shell(f"lcov -c  -b ../ -d {OBJ_DIR} -o total_unfiltered.info")
    shell(f"lcov --remove total_unfiltered.info {LCOV_EXCLUDE} -o total.info")
    shell(f"genhtml total.info -o {COVERAGE_REPORT_DIR}/")
    print("📜 Opening report...")
    shell(f"{OPEN_BIN} {COVERAGE_REPORT_DIR}/index.html")
    os.chdir("..")
