import os

from tests_runner.utils.config import COVERAGE_INFO_DIR, OBJ_DIR


def shell(command: str) -> None:
    os.system(command + ' > /dev/null')


def set_gmon_prefix(file_name: str) -> None:
    os.environ["GMON_OUT_PREFIX"] = file_name + ".gmon.out"


def setup_coverage_dir() -> None:
    shell(f"rm -rf {COVERAGE_INFO_DIR}")
    shell(f"mkdir -p {COVERAGE_INFO_DIR}")


def prepare_coverage_report() -> None:
    os.chdir("tests")
    print("🤖 Preparing coverage report...")
    shell(f"lcov -c  -b ../ -d {OBJ_DIR} -o total_unfiltered.info")
    shell("lcov --remove total_unfiltered.info '/usr/include/*' '/usr/lib/*' -o total.info")
    shell(f"genhtml total.info -o {COVERAGE_INFO_DIR}/")
    print("📜 Opening report...")
    shell(f"xdg-open {COVERAGE_INFO_DIR}/index.html")
    os.chdir("..")
