from typing import List

import os
import sys
import shutil
import subprocess


def is_non_system_dylib(dylib_path: str) -> bool:
    return not dylib_path.startswith('/usr/lib') and not dylib_path.startswith('/System/Library')


def get_child_dylib_abs_path(parent_dylib_dir: str, child_dylib: str) -> str:
    if child_dylib.startswith('@loader_path'):
        return os.path.join(parent_dylib_dir, *os.path.split(child_dylib)[1:])

    if child_dylib.startswith('@rpath'):
        rpath = os.path.join('..', 'lib')  # assuming rpath is always @loader_path/../lib
        child_dylib_rel_path = os.path.join(rpath, *os.path.split(child_dylib)[1:])
        child_dylib_path = os.path.join(parent_dylib_dir, child_dylib_rel_path)
        return os.path.abspath(child_dylib_path)

    return child_dylib


def get_child_dylibs(exec_or_dylib_path: str) -> List[str]:
    otool_subp = subprocess.run(
        ['otool', '-L', exec_or_dylib_path], stdout=subprocess.PIPE, check=True
    )

    otool_output = otool_subp.stdout.decode('utf-8')

    child_dylibs = [line.split()[0] for line in otool_output.splitlines()[1:]]

    non_system_child_dylibs = filter(is_non_system_dylib, child_dylibs)

    return list(non_system_child_dylibs)


def rewire_dylib(exec_path: str, original_dylib_path: str, new_dylib_path: str) -> None:
    subprocess.run(
        ['install_name_tool', '-change', original_dylib_path, new_dylib_path, exec_path],
        check=True
    )


def subtract_list(lhs: List[str], rhs: List[str]) -> List[str]:
    return [x for x in lhs if x not in rhs]


class DylibPackager:
    def __init__(self, exec_path: str, libs_dir_name: str = 'libs') -> None:
        self._libs_dir = os.path.join(os.path.dirname(exec_path), libs_dir_name)
        self._exec_path = exec_path
        self._libs_dir_name = libs_dir_name
        self._depth = -1
        self._walked_libs: List[str] = []

        shutil.rmtree(self._libs_dir, ignore_errors=True)
        os.makedirs(self._libs_dir)

    def _walk(self, original_dylib_path: str, copied_dylib_path: str) -> None:
        child_dylibs = get_child_dylibs(original_dylib_path)
        child_dylibs = subtract_list(child_dylibs, self._walked_libs)
        original_dylib_dir = os.path.dirname(original_dylib_path)

        self._depth += 1

        print('  '*self._depth + original_dylib_path + ':')

        for dylib_path in child_dylibs:
            self._walked_libs.append(dylib_path)

            dylib_base_name = os.path.basename(dylib_path)
            abs_dylib_path = get_child_dylib_abs_path(original_dylib_dir, dylib_path)

            new_dylib_path = os.path.join(self._libs_dir, dylib_base_name)

            shutil.copy2(abs_dylib_path, new_dylib_path)

            rewire_dylib_path = os.path.join(
                '@loader_path',
                *(['..']*self._depth),
                self._libs_dir_name,
                dylib_base_name
            )

            print(('  '*self._depth)+'rewire', dylib_path, '=>', new_dylib_path)
            rewire_dylib(copied_dylib_path, dylib_path, rewire_dylib_path)

            self._walk(abs_dylib_path, new_dylib_path)

        self._depth -= 1

    def walk(self) -> None:
        self._walk(self._exec_path, self._exec_path)


if __name__ == '__main__':
    dylib_packager = DylibPackager(sys.argv[1])
    dylib_packager.walk()
