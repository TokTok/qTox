#!/usr/bin/env python3

import os
import re
import requests
import subprocess
from typing import Any


SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
GIT_BASE_DIR = os.path.dirname(SCRIPT_DIR)

GITHUB_API_URL = os.getenv("GITHUB_API_URL") or "https://api.github.com"
GITHUB_TOKEN = os.getenv("GITHUB_TOKEN")

if GITHUB_TOKEN:
    print("Authorization with GITHUB_TOKEN")
else:
    print("Unauthorized (low rate limit applies)")
    print("Set GITHUB_TOKEN to increase the rate limit")


github_api_requests: list[str] = []

def github_api(url: str, params: dict[str, str]) -> dict:
    """Calls the GitHub API with the given URL (GET only).

    Authorization is done with the GITHUB_TOKEN environment variable if it is set.
    """
    headers: dict[str, str] = {}
    if GITHUB_TOKEN:
        headers["Authorization"] = f"token {GITHUB_TOKEN}"
    github_api_requests.append(f"GET {GITHUB_API_URL}{url}")
    response = requests.get(f"{GITHUB_API_URL}{url}", headers=headers, params=params)
    response.raise_for_status()
    return response.json()

def git_branch():
    """Calls git rev-parse --abbrev-ref HEAD to get the current branch name."""
    return subprocess.check_output(
        ["git", "rev-parse", "--abbrev-ref", "HEAD"],
        universal_newlines=True,
    ).strip()

GIT_BRANCH = git_branch()


def github_remote(remote: str) -> str:
    url = subprocess.check_output(
        ["git", "remote", "get-url", remote],
        universal_newlines=True,
    ).strip()
    if "github.com" in url:
        return url.split(":")[-1].replace(".git", "")
    raise subprocess.CalledProcessError(1, "git remote get-url", url)


def github_username() -> str:
    """Returns the GitHub username for the current repository."""
    return github_remote("origin").split("/")[0]


GITHUB_USERNAME = github_username()
GITHUB_REPOSITORY = os.getenv("GITHUB_REPOSITORY") or github_remote("upstream")


def github_pr_number() -> int:
    """Calls the GitHub API to get the PR number for the current branch.

    Requires the GITHUB_API_URL and GITHUB_REF environment variables to be set.
    """
    return github_api(f"/repos/{GITHUB_REPOSITORY}/pulls", {"head": f"{GITHUB_USERNAME}:{GIT_BRANCH}"})[0]["number"]


def github_ref_name() -> str:
    return os.getenv("GITHUB_REF_NAME") or f"{github_pr_number()}/merge"


GITHUB_REF_NAME = github_ref_name()


def github_pr() -> dict[str, Any]:
    """Calls the GitHub API to get the current PR object."""
    return github_api(f"/repos/{GITHUB_REPOSITORY}/pulls/{GITHUB_REF_NAME.split('/')[0]}", {})

def github_pr_branch() -> str:
    """Calls the GitHub API to get the branch name for the current PR."""
    return github_pr()["head"]["ref"]


GITHUB_PR_BRANCH = github_pr_branch()


def github_base_ref() -> str:
    """Calls the GitHub API to get the base branch for the current PR."""
    return github_pr()["base"]["ref"]


GITHUB_BASE_REF = os.getenv("GITHUB_BASE_REF") or github_base_ref()


def print_check_start(name: str, description: str) -> None:
    """Prints a colorful message indicating that a check is starting.

    Does not print a newline at the end.

    Looks roughly like:
       [ .... ] Check name (requirement description)
    """
    print(f"\033[1;34m[ .... ]\033[0m {name} {description}", end="")


def print_check_end(name: str, description: str, success: bool) -> None:
    """Prints a colorful message indicating that a check has finished.

    Now it prints a newline at the end to get ready for the next check.

    Looks roughly like:
       [  OK  ] Check name (result description)
    """
    status = " \033[1;32mOK\033[0m " if success else "\033[1;31mFAIL\033[0m"
    print(f"\r\033[1;34m[ {status} \033[1;34m]\033[0m {name} {description}")


def padded(text: str, length: int) -> str:
    """Pads the text with spaces to the given length."""
    return f"{text}{' ' * (length - len(text))}"


class Checker:
    """A class to run checks and print colorful messages for the user."""

    def __init__(self, failures: list[str], name: str, description: str) -> None:
        self.failures = failures
        self.name = name
        self.description = f"({description})"
        self.done = False

    def __enter__(self) -> "Checker":
        print_check_start(self.name, self.description)
        return self

    def ok(self, description: str) -> None:
        print_check_end(self.name, padded(f"({description})", len(self.description)), True)
        self.done = True

    def fail(self, description: str) -> None:
        print_check_end(self.name, padded(f"({description})", len(self.description)), False)
        self.failures.append(self.name)
        self.done = True

    def __exit__(self, exc_type, exc_value, traceback) -> None:
        if not self.done:
            self.fail("The check did not complete")


def github_weblate_prs() -> list[tuple[str, str]]:
    """List all the open Weblate PRs.

    Weblate PRs are those who are opened by the Weblate bot called "weblate".
    """
    return [(pr["title"], pr["html_url"]) for pr in github_api(f"/repos/{GITHUB_REPOSITORY}/pulls", {}) if pr["user"]["login"] == "weblate"]


def check_github_weblate_prs(failures: list[str]) -> None:
    """Check that all Weblate PRs are merged."""
    with Checker(failures, "Weblate PRs", "All Weblate PRs should be merged") as check:
        weblate_prs = github_weblate_prs()
        if weblate_prs:
            check.fail("Some Weblate PRs are still open")
            for pr in weblate_prs:
                print(f"  - {pr[0]} ({pr[1]})")
        else:
            check.ok("All Weblate PRs are merged")

# Check if $GIT_BASE_DIR/../dockerfiles exists. If not, clone
# https://github.com/TokTok/dockerfiles.git into .ci-scripts/dockerfiles.
DOCKERFILES_DIR = os.path.join(os.path.dirname(GIT_BASE_DIR), "dockerfiles")
if not os.path.isdir(DOCKERFILES_DIR):
    DOCKERFILES_DIR = os.path.join(SCRIPT_DIR, "dockerfiles")
    subprocess.run(
        ["git", "clone", "--depth=1", "https://github.com/TokTok/dockerfiles.git", DOCKERFILES_DIR],
        check=True,
    )

def check_flathub_descriptor_dependencies(failures: list[str]) -> None:
    """Runs flatpak/update_flathub_descriptor_dependencies.py and checks if it made any changes."""
    with Checker(failures, "Flathub dependencies", "Update flathub descriptor dependencies") as check:
        flathub_manifest_path = os.path.join(GIT_BASE_DIR, "flatpak", "io.github.qtox.qTox.json")
        subprocess.run([
                "flatpak/update_flathub_descriptor_dependencies.py",
                "--flathub-manifest", flathub_manifest_path,
                "--output", flathub_manifest_path,
                "--download-files-path", os.path.join(DOCKERFILES_DIR, "qtox", "download"),
                "--git-tag", f"v{GITHUB_PR_BRANCH}",
            ],
            check=True,
            cwd=GIT_BASE_DIR,
        )
        if subprocess.run(["git", "diff", "--exit-code", flathub_manifest_path]).returncode:
            check.fail("The flathub descriptor dependencies have changed")
            # Reset the changes to the flathub descriptor.
            subprocess.run(["git", "checkout", flathub_manifest_path], check=True)
        else:
            check.ok("The flathub descriptor dependencies are up-to-date")


def check_toxcore_version(failures: list[str]) -> None:
    """Check that qtox/download/download_toxcore.sh is up-to-date.

    We get the latest release version of TokTok/c-toxcore from GitHub and
    compare it to the one in the script (which has a line like TOXCORE_VERSION=0.2.20).
    """
    with Checker(failures, "Toxcore version", "Check if the toxcore version is up-to-date") as check:
        download_toxcore_path = os.path.join(DOCKERFILES_DIR, "qtox", "download", "download_toxcore.sh")
        with open(download_toxcore_path) as f:
            found = re.search(r"^TOXCORE_VERSION=(.*)$", f.read(), re.MULTILINE)
            if not found:
                check.fail("Could not find the toxcore version in the download script")
                return
            toxcore_version = found.group(1)

        latest_toxcore_version = github_api("/repos/TokTok/c-toxcore/releases/latest", {})["tag_name"]
        if f"v{toxcore_version}" == latest_toxcore_version:
            check.ok(f"The toxcore version is up-to-date: {toxcore_version}")
        else:
            check.fail(f"The toxcore version is outdated: {toxcore_version} (latest: {latest_toxcore_version})")


def check_package_versions(failures: list[str]) -> None:
    """Runs tools/update-versions.sh $GITHUB_PR_BRANCH and checks if it made any changes."""
    with Checker(failures, "Package versions", "README and package versions should be up-to-date") as check:
        subprocess.run(
            ["tools/update-versions.sh", GITHUB_PR_BRANCH],
            check=True,
            cwd=GIT_BASE_DIR,
        )
        if subprocess.run(["git", "diff", "--exit-code"]).returncode:
            check.fail("The package versions need to be updated")
            # Reset the changes to the README and package versions.
            subprocess.run([
                "git", "checkout", "README.md",
                "macos/Info.plist", "res/io.github.qtox.qTox.appdata.xml",
                "windows/qtox.nsi", "windows/qtox64.nsi",
            ], cwd=GIT_BASE_DIR, check=True)
        else:
            check.ok("The package versions are up-to-date")


def check_no_version_changes(failures: list[str]) -> None:
    """Check that no version changes are made in a non-release PR.

    Diff res/io.github.qtox.qTox.appdata.xml against $GITHUB_BASE_REF and check
    if there's a line starting with "+" or "-" that contains a version number.

    Example:
    -  <release version="1.18.0-rc.3" date="2024-12-29"/>
    +  <release version="1.18.0" date="2024-12-29"/>
    """
    with Checker(failures, "No version changes", "No version changes should be made in a non-release PR") as check:
        diff = subprocess.check_output(
            ["git", "diff", GITHUB_BASE_REF, "--", "res/io.github.qtox.qTox.appdata.xml"],
            cwd=GIT_BASE_DIR,
            universal_newlines=True,
        )
        if re.search(r"^[+-]\s+<release version=\"", diff, re.MULTILINE):
            check.fail("Version changes are not allowed in a non-release PR")
        else:
            check.ok("No version changes were made")


def main() -> None:
    """Main entry point."""
    print("GIT_BASE_DIR:     ", GIT_BASE_DIR)
    print("GIT_BRANCH:       ", GIT_BRANCH)
    print("GITHUB_API_URL:   ", GITHUB_API_URL)
    print("GITHUB_BASE_REF:  ", GITHUB_BASE_REF)
    print("GITHUB_PR_BRANCH: ", GITHUB_PR_BRANCH)
    print("GITHUB_REF_NAME:  ", GITHUB_REF_NAME)
    print("GITHUB_REPOSITORY:", GITHUB_REPOSITORY)
    print("GITHUB_USERNAME:  ", GITHUB_USERNAME)

    print("\nRunning checks...\n")

    failures: list[str] = []

    # If the PR branch looks like a version number, do checks for a release PR.
    if re.match(r"^v?\d+\.\d+\.\d+", GITHUB_PR_BRANCH):
        print("This is a release PR.\n")
        check_github_weblate_prs(failures)
        check_flathub_descriptor_dependencies(failures)
        check_toxcore_version(failures)
        check_package_versions(failures)
    else:
        print("This is not a release PR.\n")
        check_no_version_changes(failures)

    print(f"\nDebug: {len(github_api_requests)} GitHub API requests made")

    if failures:
        print("\nSome checks failed:")
        for failure in failures:
            print(f"  - {failure}")
        exit(1)


if __name__ == "__main__":
    main()
