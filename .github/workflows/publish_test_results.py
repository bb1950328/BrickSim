# usage: python2 publish_test_results.py /path/to/catch2_output.xml github_token git_commit_hash

import github
import pathlib
import xml.etree.ElementTree as ET
from typing import List

import sys



class Expression(object):
    type: str
    filename: str
    line: int
    original: str
    expanded: str


class TestCaseResult(object):
    name: str
    filename: str
    line: int
    durationInSeconds: float
    success: bool
    expressions: List[Expression] = []


_, XML_FILE_NAME, GITHUB_TOKEN, CURRENT_COMMIT_HASH = sys.argv

REPO_ID = "bb1950328/BrickSim"
REPO_BASE_URL = f"https://www.github.com/{REPO_ID}"

BASE_PATH = str(pathlib.Path(__file__).parent.parent.parent.absolute())


def format_duration(seconds: float) -> str:
    if seconds < 0.001:
        return f"{seconds * 1_000_000:.2f}µs"
    elif seconds < 1:
        return f"{seconds * 1_000:.2f}ms"
    else:
        return f"{seconds}s"


def get_relative_file_path(absolute_path: str) -> str:
    return pathlib.Path(absolute_path).relative_to(BASE_PATH)


def to_github_permalink(relative_file_path: str, line: int) -> str:
    return f"[`{relative_file_path}:{line}`]({REPO_BASE_URL}/blob/{CURRENT_COMMIT_HASH}/{relative_file_path}#L{line})"


def format_successful_row(case: TestCaseResult) -> str:
    return f"  | {case.name} | {to_github_permalink(case.filename, case.line)} | {format_duration(case.durationInSeconds)} |"


def format_failed_row(case: TestCaseResult) -> str:
    return f"  | {case.name} | {to_github_permalink(case.filename, case.line)} | `{case.expressions[0].type}({case.expressions[0].original})` | `{case.expressions[0].type}({case.expressions[0].expanded})` | {format_duration(case.durationInSeconds)} |"


if __name__ == '__main__':
    tree = ET.parse(XML_FILE_NAME)

    cases: List[TestCaseResult] = []
    successfulCases: List[TestCaseResult] = []
    failedCases: List[TestCaseResult] = []

    numSuccessfulAssertions = -1
    numFailedAssertions = -1
    numSuccessfulCases = -1
    numFailedCases = -1

    for group in tree.getroot():
        if group.tag == "Group":
            for testCase in group:
                if testCase.tag != "TestCase":
                    continue
                c = TestCaseResult()
                c.name = testCase.attrib["name"]
                c.filename = get_relative_file_path(testCase.attrib["filename"])
                c.line = int(testCase.attrib["line"])
                cases.append(c)

                for subElement in testCase:
                    if subElement.tag == "OverallResult":
                        c.success = subElement.attrib["success"] == "true"
                        c.durationInSeconds = float(subElement.attrib["durationInSeconds"])
                    elif subElement.tag == "Expression":
                        e = Expression()
                        e.type = subElement.attrib["type"]
                        e.filename = get_relative_file_path(subElement.attrib["filename"])
                        e.line = int(subElement.attrib["line"])
                        c.expressions.append(e)

                        for eSub in subElement:
                            if eSub.tag == "Original":
                                e.original = eSub.text.strip()
                            elif eSub.tag == "Expanded":
                                e.expanded = eSub.text.strip()
                (successfulCases if c.success else failedCases).append(c)
        elif group.tag == "OverallResults":
            numSuccessfulAssertions = int(group.attrib["successes"])
            numFailedAssertions = int(group.attrib["failures"])
        elif group.tag == "OverallResultsCases":
            numSuccessfulCases = int(group.attrib["successes"])
            numFailedCases = int(group.attrib["failures"])

    successfulCases.sort(key=lambda res: f"{res.filename}:{res.line:09}")
    failedCases.sort(key=lambda res: f"{res.filename}:{res.line:09}")

    totalDurationInSeconds = sum(c.durationInSeconds for c in cases)

    textLines = ["# Catch2 Test Results"]
    textLines += [
        f"<details>",
        f"  <summary>✔ {numSuccessfulCases} case{'' if numSuccessfulCases == 1 else 's'}, {numSuccessfulAssertions} assertion{'' if numSuccessfulAssertions == 1 else 's'}</summary>",
        f"  ",
        f"  | 🏷️ Name | 📄 File | ⏱ Duration |",
        f"  | ------- | ------- | ----------- |",
        *[format_successful_row(c) for c in successfulCases],
        f"</details>",
        f"",
    ]
    textLines += [
        f"<details>",
        f"  <summary>❌ {numFailedCases} case{'' if numFailedCases == 1 else 's'}, {numFailedAssertions} assertion{'' if numFailedAssertions == 1 else 's'}</summary>",
        f"  ",
        f"  | 🏷️ Name | 📄 File | Original | Expanded | ⏱ Duration |",  # todo don't show the table when there are 0 cases
        f"  | ------- | ------- | -------- | -------- | ----------- |",
        *[format_failed_row(c) for c in failedCases],
        f"</details>",
        f"",
    ]

    textLines += [
        f"⏱ Total Duration: {format_duration(totalDurationInSeconds)}, commit [`{CURRENT_COMMIT_HASH[:10]}`]({REPO_BASE_URL}/tree/{CURRENT_COMMIT_HASH})",
    ]

    text = "\n".join(textLines)
    print(text)

    gh = github.Github(GITHUB_TOKEN)
    repo = gh.get_repo(REPO_ID)
    commit = repo.get_commit(CURRENT_COMMIT_HASH)
    commit.create_comment(text)
