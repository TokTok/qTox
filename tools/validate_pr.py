#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2024-2025 The TokTok team
import argparse
import os
import re
import subprocess  # nosec
from dataclasses import dataclass
from functools import cache as memoize

import update_changelog
from lib import git
from lib import github
from lib import stage


@dataclass
class Config:
    commit: bool


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="""
    Run a bunch of checks to validate a PR. This script is meant to be run in a
    GitHub Actions workflow, but can also be run locally.
    """)
    parser.add_argument(
        "--commit",
        action=argparse.BooleanOptionalAction,
        help="Stage changes with git add (no commit yet)",
        default=False,
    )
    return parser.parse_args()


SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
GIT_BASE_DIR = os.path.dirname(SCRIPT_DIR)


def github_weblate_prs() -> list[tuple[str, str]]:
    """List all the open Weblate PRs.

    Weblate PRs are those who are opened by the Weblate bot called "weblate".
    """
    return [(pr["title"], pr["html_url"])
            for pr in github.api(f"/repos/{github.repository()}/pulls")
            if pr["user"]["login"] == "weblate"]


def check_github_weblate_prs(failures: list[str]) -> None:
    """Check that all Weblate PRs are merged."""
    with stage.Stage("Weblate PRs", "All Weblate PRs should be merged",
                     failures) as check:
        weblate_prs = github_weblate_prs()
        if weblate_prs:
            check.fail("Some Weblate PRs are still open")
            for pr in weblate_prs:
                print(f"  - {pr[0]} ({pr[1]})")
        else:
            check.ok("All Weblate PRs are merged")


@memoize
def dockerfiles_dir() -> str:
    # Check if $GIT_BASE_DIR/../dockerfiles exists. If not, clone
    # https://github.com/TokTok/dockerfiles.git into .ci-scripts/dockerfiles.
    repo_dir = os.path.join(os.path.dirname(GIT_BASE_DIR), "dockerfiles")
    if not os.path.isdir(repo_dir):
        repo_dir = os.path.join(SCRIPT_DIR, "dockerfiles")
    if not os.path.isdir(repo_dir):
        subprocess.run(  # nosec
            [
                "git",
                "clone",
                "--depth=1",
                "https://github.com/TokTok/dockerfiles.git",
                repo_dir,
            ],
            check=True,
        )
    return repo_dir


def has_diff(config: Config, *files: str) -> bool:
    """Check if there are any changes in the git repository.

    If `config.commit` is True, the diff will be quiet.
    """
    quiet = ["--quiet"] if config.commit else []
    return (subprocess.run(  # nosec
        ["git", "diff", *quiet, "--exit-code", *files]).returncode != 0)


def check_flathub_descriptor_dependencies(failures: list[str],
                                          config: Config) -> None:
    """Runs flatpak/update_flathub_descriptor_dependencies.py and checks if it made any changes."""
    with stage.Stage("Flathub dependencies",
                     "Update flathub descriptor dependencies",
                     failures) as check:
        flathub_manifest_path = os.path.join(GIT_BASE_DIR, "flatpak",
                                             "com.github.TokTok.qTox.json")
        subprocess.run(  # nosec
            [
                "flatpak/update_flathub_descriptor_dependencies.py",
                "--quiet",
                "--flathub-manifest",
                flathub_manifest_path,
                "--output",
                flathub_manifest_path,
                "--download-files-path",
                os.path.join(dockerfiles_dir(), "qtox", "download"),
                "--git-tag",
                github.head_ref().removeprefix(
                    f"{git.RELEASE_BRANCH_PREFIX}/"),
            ],
            check=True,
            cwd=GIT_BASE_DIR,
        )
        if has_diff(config, flathub_manifest_path):
            if config.commit:
                git.add(flathub_manifest_path)
                check.ok(
                    "The flathub descriptor dependencies have been updated")
            else:
                check.fail("The flathub descriptor dependencies have changed")
                # Reset the changes to the flathub descriptor.
                git.revert(flathub_manifest_path)
        else:
            check.ok("The flathub descriptor dependencies are up-to-date")


def check_toxcore_version(failures: list[str]) -> None:
    """Check that qtox/download/download_toxcore.sh is up-to-date.

    We get the latest release version of TokTok/c-toxcore from GitHub and
    compare it to the one in the script (which has a line like TOXCORE_VERSION=0.2.20).
    """
    with stage.Stage("Toxcore version",
                     "Check if the toxcore version is up-to-date",
                     failures) as check:
        download_toxcore_path = os.path.join(dockerfiles_dir(), "qtox",
                                             "download", "download_toxcore.sh")
        with open(download_toxcore_path) as f:
            found = re.search(r"^TOXCORE_VERSION=(.*)$", f.read(),
                              re.MULTILINE)
            if not found:
                check.fail(
                    "Could not find the toxcore version in the download script"
                )
                return
            toxcore_version = found.group(1)

        latest_toxcore_version = github.api(
            "/repos/TokTok/c-toxcore/releases/latest")["tag_name"]
        if f"v{toxcore_version}" == latest_toxcore_version:
            check.ok(f"The toxcore version is up-to-date: {toxcore_version}")
        else:
            check.fail(
                f"The toxcore version is outdated: {toxcore_version} (latest: {latest_toxcore_version})"
            )


def check_package_versions(failures: list[str], config: Config) -> None:
    """Runs tools/update-versions.sh ${GITHUB_HEAD_REF/release\\/v/} and checks if it made any changes."""
    with stage.Stage("Package versions",
                     "README and package versions should be up-to-date",
                     failures) as check:
        subprocess.run(  # nosec
            [
                "tools/update-versions.sh",
                github.head_ref().removeprefix(
                    f"{git.RELEASE_BRANCH_PREFIX}/v"),
            ],
            check=True,
            cwd=GIT_BASE_DIR,
        )
        files = (
            "README.md",
            "macos/Info.plist",
            "res/com.github.TokTok.qTox.appdata.xml",
            "windows/qtox.nsi",
            "windows/qtox64.nsi",
        )
        if has_diff(config, *files):
            if config.commit:
                git.add(".")
                check.ok("The package versions have been updated")
            else:
                check.fail("The package versions need to be updated")
                # Reset the changes to the README and package versions.
                git.revert(*files)
        else:
            check.ok("The package versions are up-to-date")


def check_no_version_changes(failures: list[str]) -> None:
    """Check that no version changes are made in a non-release PR.

    Diff res/com.github.TokTok.qTox.appdata.xml against $GITHUB_BASE_BRANCH and
    check if there's a line starting with "+" or "-" that contains a version
    number.

    Example:
    -  <release version="1.18.0-rc.3" date="2024-12-29"/>
    +  <release version="1.18.0" date="2024-12-29"/>
    """
    with stage.Stage(
            "No version changes",
            "No version changes should be made in a non-release PR",
            failures,
    ) as check:
        diff = subprocess.check_output(  # nosec
            [
                "git",
                "diff",
                github.base_branch(),
                "--",
                "res/com.github.TokTok.qTox.appdata.xml",
            ],
            cwd=GIT_BASE_DIR,
            universal_newlines=True,
        )
        minus = re.findall(r"^-[^<]+<release version=\"(.*)\" date", diff,
                           re.MULTILINE)
        plus = re.findall(r"^\+[^<]+<release version=\"(.*)\" date", diff,
                          re.MULTILINE)
        if minus and plus:
            check.fail("Version changes are not allowed"
                       f" in a non-release PR ({minus[0]} -> {plus[0]})")
        elif minus or plus:
            check.fail(
                "Removal or addition of a version number is not allowed"
                f" in a non-release PR ({minus[0] if minus else plus[0]})")
        else:
            check.ok("No version changes were made")


def check_changelog(failures: list[str], config: Config) -> None:
    """Check that the changelog is up-to-date."""
    with stage.Stage("Changelog", "The changelog should be up-to-date",
                     failures) as check:
        update_changelog.main()
        if has_diff(config, "CHANGELOG.md"):
            if config.commit:
                git.add("CHANGELOG.md")
                check.ok("The changelog has been updated")
            else:
                check.fail("The changelog needs to be updated")
                # Reset the changes to the changelog.
                subprocess.run(  # nosec
                    ["git", "checkout", "CHANGELOG.md"],
                    cwd=GIT_BASE_DIR,
                    check=True,
                )
        else:
            check.ok("The changelog is up-to-date")


def main(config: Config) -> None:
    """Main entry point."""
    print("GIT_BASE_DIR:       ", GIT_BASE_DIR)
    print("GITHUB_ACTOR:       ", github.actor())
    print("GITHUB_API_URL:     ", github.api_url())
    print("GITHUB_BASE_REF:    ", github.base_ref())
    print("GITHUB_BASE_BRANCH: ", github.base_branch())
    print("GITHUB_HEAD_REF:    ", github.head_ref())
    print("GITHUB_PR_BRANCH:   ", github.pr_branch())
    print("GITHUB_REF_NAME:    ", github.ref_name())
    print("GITHUB_REPOSITORY:  ", github.repository())

    print("\nRunning checks...\n")

    failures: list[str] = []

    # If the PR branch looks like a version number, do checks for a release PR.
    if re.match(git.RELEASE_BRANCH_REGEX, github.head_ref()):
        print("This is a release PR.\n")
        check_github_weblate_prs(failures)
        check_flathub_descriptor_dependencies(failures, config)
        check_toxcore_version(failures)
        check_package_versions(failures, config)
    else:
        print(
            f"This is not a release PR ({git.RELEASE_BRANCH_REGEX.pattern}).\n"
        )
        check_no_version_changes(failures)

    check_changelog(failures, config)

    print(f"\nDebug: {len(github.api_requests)} GitHub API requests made")

    if failures:
        print("\nSome checks failed:")
        for failure in failures:
            print(f"  - {failure}")
        exit(1)


if __name__ == "__main__":
    main(Config(**vars(parse_args())))
