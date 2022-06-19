from typing import List, Generator, Optional

import os
import glob

from contextlib import contextmanager


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


def dump_string_to_file(file_name: str, content: str) -> None:
    with open(file_name, "w") as file:
        file.write(content)


def string_from_file(file_name: str) -> str:
    with open(file_name, "r") as file:
        return file.read()


@contextmanager
def dirctx(path: Optional[str]) -> Generator[None, None, None]:
    saved_path = os.getcwd()

    if path is not None:
        os.chdir(path)

    yield None

    os.chdir(saved_path)
