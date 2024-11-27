#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2021 by The qTox Project Contributors
# Copyright © 2024 The TokTok team
import argparse
import json
import pathlib
import re
import subprocess  # nosec
import tempfile
import unittest
from typing import Any
from typing import Optional

QTOX_ROOT = pathlib.Path(__file__).parent.parent
TOKTOK_ROOT = QTOX_ROOT.parent
DOWNLOAD_FILE_PATHS = TOKTOK_ROOT / "dockerfiles" / "qtox" / "download"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="""
    Update dependencies of a flathub manifest to match the versions used by
    qTox. This script will iterate over all known dependencies in the manifest
    and replace their tags with the ones specified by our download_xxx.sh
    scripts. The commit hash for the tag will be replaced with whatever is
    currently in the git remote
    """)
    parser.add_argument(
        "--flathub-manifest",
        help="Path to flathub manifest",
        required=True,
        dest="flathub_manifest_path",
    )
    parser.add_argument(
        "--output",
        help="Output manifest path",
        required=True,
        dest="output_manifest_path",
    )
    parser.add_argument(
        "--download-files-path",
        help="Path to the dockerfiles/qtox/download directory",
        required=False,
        default=DOWNLOAD_FILE_PATHS,
        dest="download_files_path",
    )
    parser.add_argument(
        "--git-tag",
        help="Git tag to use for the qTox version",
        required=False,
        default=None,
        dest="git_tag",
    )
    return parser.parse_args()


PRINT_VERSION_SCRIPT = """
download_verify_extract_tarball() {
    echo "URL: $1"
    echo "HASH: $2"
}
"""


def find_version(download_script_path: pathlib.Path) -> tuple[str, str]:
    """
    Find the version and hash specified for a given dependency by parsing its
    download script.

    Returns a tuple of (version, hash).
    """
    with open(download_script_path) as f:
        script_content = "".join(
            re.sub(r"^source.*", PRINT_VERSION_SCRIPT, line)
            for line in f.readlines())

    # Run bash script in a subshell to extract the version
    version_output = subprocess.run(  # nosec
        ["bash", "-c", script_content],
        check=True,
        stdout=subprocess.PIPE,
    ).stdout.decode()

    # Extract the version and hash from the output
    matches = re.match(r"URL: (.*)\nHASH: (.*)", version_output, re.MULTILINE)
    if matches is None:
        raise ValueError(
            "Failed to extract version and hash from download script")

    return matches.group(1), matches.group(2)


class FindVersionTest(unittest.TestCase):

    def test_version_parsing(self) -> None:
        # Create a dummy download script and check that we can extract the version from it
        with tempfile.TemporaryDirectory() as d:
            sample_download_script = """
            #!/bin/bash

            source "$(dirname "$0")"/common.sh

            TEST_VERSION=1.2.3
            TEST_HASH=:)

            download_verify_extract_tarball \
                "https://test_site.com/$TEST_VERSION" \
                "$TEST_HASH"
            """

            sample_download_script_path = pathlib.Path(d) / "/test_script.sh"
            with open(sample_download_script_path, "w") as f:
                f.write(sample_download_script)

            self.assertEqual(find_version(sample_download_script_path),
                             ("1.2.3", ":)"))


def load_flathub_manifest(flathub_manifest_path: str) -> Any:
    with open(flathub_manifest_path) as f:
        return json.load(f)


def commit_from_tag(url: str, tag: str) -> str:
    git_output = subprocess.run(  # nosec
        ["git", "ls-remote", url, f"{tag}^{{}}"],
        check=True,
        stdout=subprocess.PIPE,
    )
    commit = git_output.stdout.split(b"\t")[0]
    return commit.decode()


class CommitFromTagTest(unittest.TestCase):

    def test_commit_from_tag(self) -> None:
        self.assertEqual(
            commit_from_tag(str(QTOX_ROOT), "v1.17.3"),
            "c0e9a3b79609681e5b9f6bbf8f9a36cb1993dc5f",
        )


def get_git_tag() -> str:
    git_output = subprocess.run(  # nosec
        ["git", "describe", "--tags", "--abbrev=0"],
        check=True,
        stdout=subprocess.PIPE,
    )
    return git_output.stdout.decode().strip()


def update_archive_source(
    module: dict[str, Any],
    source: tuple[str, str],
) -> None:
    url, sha256 = source
    module_source = module["sources"][0]
    module_source["url"] = url
    module_source["sha256"] = sha256


def update_git_source(module: dict[str, Any], tag: str) -> None:
    module_source = module["sources"][0]
    module_source["tag"] = tag
    module_source["commit"] = commit_from_tag(
        module_source["url"],
        module_source["tag"],
    )


def main(flathub_manifest_path: str, output_manifest_path: str, download_files_path: str, git_tag: Optional[str]) -> None:
    flathub_manifest = load_flathub_manifest(flathub_manifest_path)

    download_files_dir = pathlib.Path(download_files_path)
    sqlcipher_version = find_version(download_files_dir /
                                     "download_sqlcipher.sh")
    sodium_version = find_version(download_files_dir / "download_sodium.sh")
    toxcore_version = find_version(download_files_dir / "download_toxcore.sh")
    qTox_version = git_tag or get_git_tag()

    for module in flathub_manifest["modules"]:
        if module["name"] == "sqlcipher":
            update_archive_source(module, sqlcipher_version)
        elif module["name"] == "libsodium":
            update_archive_source(module, sodium_version)
        elif module["name"] == "c-toxcore":
            update_archive_source(module, toxcore_version)
        elif module["name"] == "qTox":
            update_git_source(module, qTox_version)

    orig = load_flathub_manifest(output_manifest_path)
    if json.dumps(flathub_manifest) == json.dumps(orig):
        print("No changes needed")
    else:
        print("Changes detected, writing to", output_manifest_path)
        with open(output_manifest_path, "w") as f:
            json.dump(flathub_manifest, f, indent=2)
            f.write("\n")


if __name__ == "__main__":
    main(**vars(parse_args()))
