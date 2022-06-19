from __future__ import annotations

from typing import List, Optional

from difflib import ndiff


class ResultType:
    PASSED = 0
    FAILED = 1
    TIMEDOUT = 2
    SKIPPED = 3
    INVALID = 4


class Result:
    def __init__(self, result_type: int,
                 output: Optional[str], expected_output: Optional[str]) -> None:
        self.result_type = result_type
        self.output = output
        self.expected = expected_output

    def diff(self) -> str:
        if self.output is None or self.expected is None:
            return ""

        str_diff = ndiff(self.output.splitlines(keepends=True),
                         self.expected.splitlines(keepends=True))
        return "".join(str_diff)

    @staticmethod
    def passed(output: str) -> Result:
        return Result(ResultType.PASSED, output, None)

    @staticmethod
    def failed(output: str, expected_output: Optional[str] = None) -> Result:
        return Result(ResultType.FAILED, output, expected_output)

    @staticmethod
    def timedout() -> Result:
        return Result(ResultType.TIMEDOUT, None, None)

    @staticmethod
    def skipped() -> Result:
        return Result(ResultType.SKIPPED, None, None)

    @staticmethod
    def invalid(reason: Optional[str] = None) -> Result:
        return Result(ResultType.INVALID, reason, None)

    @property
    def has_failed(self) -> bool:
        return self.result_type == ResultType.FAILED

    @property
    def has_passed(self) -> bool:
        return self.result_type == ResultType.PASSED

    @property
    def has_timedout(self) -> bool:
        return self.result_type == ResultType.TIMEDOUT


class ResultPrinter:
    exit_code = 0
    _passed: List[str] = []
    _failed: List[str] = []
    _timedout: List[str] = []
    _skipped: List[str] = []
    _invalid: List[str] = []

    def __init__(self, group: str, generator: bool = False) -> None:
        self._generator = generator

        if generator:
            print(f"🚀 Generating group {group}")
        else:
            print(f"🚀 Running group {group} tests")

    def print_result(self, name: str, result: Result) -> None:

        if result.result_type == ResultType.PASSED:
            ResultPrinter._passed.append(name)

            if self._generator:
                print("    👌", name, "generated")
            else:
                print("    👌", name, "passed")

        elif result.result_type == ResultType.FAILED:
            ResultPrinter.exit_code += 1

            ResultPrinter._failed.append(name)

            print("    ❌", name, "failed\n")
            print("[Output]")
            print(result.output)

            if result.expected is None:
                return

            print("[Defined or expected output]")
            print(result.expected)
            print("[Diff]")
            print(result.diff())

        elif result.result_type == ResultType.INVALID:
            ResultPrinter.exit_code += 1

            ResultPrinter._invalid.append(name)

            print("    🤔", name, f"invalid test case, {result.output}")

        elif result.result_type == ResultType.TIMEDOUT:
            ResultPrinter._timedout.append(name)
            print("    🕒", name, "timedout")

        else:
            ResultPrinter._skipped.append(name)

    @staticmethod
    def print_summary() -> None:
        num_passed = len(ResultPrinter._passed)
        num_failed = len(ResultPrinter._failed)
        num_timedout = len(ResultPrinter._timedout)
        num_skipped = len(ResultPrinter._skipped)
        num_invalid = len(ResultPrinter._invalid)
        total = num_passed + num_failed + num_timedout + num_skipped + num_invalid

        print(f"✨ Ran {total} tests")
        print(f"    👉 {num_passed} tests passed")
        print(f"    👉 {num_failed} tests failed")
        print(f"    👉 {num_timedout} tests timedout")
        print(f"    👉 {num_skipped} tests skipped")
        print(f"    👉 {num_invalid} tests invalid")
