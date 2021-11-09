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
    def invalid(test_case: Optional[str] = None) -> TestResult:
        return TestResult(TestResultType.INVALID, test_case, None)

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

    def __init__(self, tests_set_name: str) -> None:
        self._passed: List[str] = []
        self._failed: List[str] = []
        self._timedout: List[str] = []
        self._skipped: List[str] = []
        self._invalid: List[str] = []

        print(f"🚀 Running {tests_set_name} tests")

    @property
    def total(self) -> int:
        return len(self._passed) + len(self._failed) + len(self._timedout)

    @property
    def passed(self) -> List[str]:
        return self._passed

    @property
    def failed(self) -> List[str]:
        return self._failed

    @property
    def timedout(self) -> List[str]:
        return self._timedout

    def print_result(self, name: str, result: TestResult) -> None:

        if result.test_result == TestResultType.PASSED:
            self._passed.append(name)
            print("    👌", name, "passed")

        elif result.test_result == TestResultType.FAILED:
            ResultPrinter.exit_code += 1

            self._failed.append(name)

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
            self._invalid.append(name)

            print("    🤔", name, "invalid test case\n")
            print("[Defined output]")
            print(result.output)

        elif result.test_result == TestResultType.TIMEDOUT:
            self._timedout.append(name)
            print("    🕒", name, "timedout")

        else:
            self._skipped.append(name)

    def print_summary(self) -> None:
        print(f"  ✨ Ran {self.total} tests")
        print(f"     {len(self.passed)} tests passed")
        print(f"     {len(self.failed)} tests failed")
        print(f"     {len(self.timedout)} tests timedout")
        print(f"     {len(self._skipped)} tests skipped")
        print(f"     {len(self._invalid)} tests invalid")
