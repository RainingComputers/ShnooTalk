from typing import Optional

from tests_runner.util.result import TestResult


def compare_outputs(expected_output: str, output: Optional[str]) -> TestResult:
    if expected_output == output or output is None:
        return TestResult.passed()

    return TestResult.failed(output, expected_output)
