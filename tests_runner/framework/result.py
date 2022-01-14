from __future__ import annotations

from typing import List, Optional

from difflib import ndiff


class TestResultType:
    PASSED = 0
    FAILED = 1
    TIMEDOUT = 2
    SKIPPED = 3
    INVALID = 4


class TestResult:
    def __init__(self, test_result: int,
                 output: Optional[str], expected_output: Optional[str]) -> None:
        self.test_result = test_result
        self.output = output
        self.expected = expected_output

    def diff(self) -> str:
        if self.output is None or self.expected is None:
            return ""

        str_diff = ndiff(self.output.splitlines(keepends=True),
                         self.expected.splitlines(keepends=True))
        return "".join(str_diff)

    @staticmethod
    def passed(output: str) -> TestResult:
        return TestResult(TestResultType.PASSED, output, None)

    @staticmethod
    def failed(output: str, expected_output: Optional[str] = None) -> TestResult:
        return TestResult(TestResultType.FAILED, output, expected_output)

    @staticmethod
    def timedout() -> TestResult:
        return TestResult(TestResultType.TIMEDOUT, None, None)

    @staticmethod
    def skipped() -> TestResult:
        return TestResult(TestResultType.SKIPPED, None, None)

    @staticmethod
    def invalid(reason: Optional[str] = None) -> TestResult:
        return TestResult(TestResultType.INVALID, reason, None)

    @property
    def has_failed(self) -> bool:
        return self.test_result == TestResultType.FAILED

    @property
    def has_passed(self) -> bool:
        return self.test_result == TestResultType.PASSED

    @property
    def has_timedout(self) -> bool:
        return self.test_result == TestResultType.TIMEDOUT


class ResultPrinter:
    exit_code = 0
    _passed: List[str] = []
    _failed: List[str] = []
    _timedout: List[str] = []
    _skipped: List[str] = []
    _invalid: List[str] = []

    def __init__(self, group: str) -> None:
        print(f"🚀 Running {group} tests")

    @staticmethod
    def print_result(name: str, result: TestResult) -> None:

        if result.test_result == TestResultType.PASSED:
            ResultPrinter._passed.append(name)
            print("    👌", name, "passed")

        elif result.test_result == TestResultType.FAILED:
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

        elif result.test_result == TestResultType.INVALID:
            ResultPrinter.exit_code += 1

            ResultPrinter._invalid.append(name)

            print("    🤔", name, f"invalid test case, {result.output}")

        elif result.test_result == TestResultType.TIMEDOUT:
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
