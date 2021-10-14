from typing import List

import os
import glob


def remove_if_exists(filename: str) -> None:
    if os.path.exists(filename):
        os.remove(filename)


def list_test_files() -> List[str]:
    files = [file for file in os.listdir() if file.endswith(".shtk")]
    files.sort()

    return files


def get_files(file_ext: str) -> List[str]:
    return glob.glob(f"**/*{file_ext}", recursive=True)


def remove_files(file_ext: str) -> None:
    files = get_files(file_ext)

    for file in files:
        remove_if_exists(file)
