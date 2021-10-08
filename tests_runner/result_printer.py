from difflib import ndiff


class TestResultType:
    PASSED = 0
    FAILED = 1
    TIMEDOUT = 2
    SKIPPED = 3


def print_diff(act_output, test_output):
    diff = ndiff(act_output.splitlines(keepends=True),
                 test_output.splitlines(keepends=True))

    print(''.join(diff))


class ResultPrinter:
    def __init__(self):
        self._passed = []
        self._failed = []
        self._timedout = []
        self._skipped = []

    @property
    def total(self):
        return len(self._passed) + len(self._failed) + len(self._timedout)

    @property
    def passed(self):
        return self._passed

    @property
    def failed(self):
        return self._failed

    @property
    def timedout(self):
        return self._timedout

    def print_result(self, name, test_result, output, expected_output=None):
        if test_result == TestResultType.PASSED:
            self._passed.append(name)
            print(" 👌", name, "passed")

        elif test_result == TestResultType.FAILED:
            self._failed.append(name)

            print(" ❌", name, "failed\n")
            print("[Output]")
            print(output)

            if expected_output is None:
                return

            print("[Defined or expected output]")
            print(expected_output)
            print("[Diff]")
            print_diff(output, expected_output)

        elif test_result == TestResultType.TIMEDOUT:
            self._timedout.append(name)
            print(" 🕒", name, "timedout")

        else:
            self._skipped.append(name)

    def print_summary(self):
        print(f"Ran {self.total} tests")
        print(f"{len(self.passed)} tests passed")
        print(f"{len(self.failed)} tests failed")
        print(f"{len(self.timedout)} tests timedout")
        print(f"{len(self._skipped)} tests skipped")
