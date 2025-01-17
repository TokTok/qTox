#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2024-2025 The TokTok team
import argparse
import os
import subprocess  # nosec
import tempfile
from dataclasses import dataclass

import requests
from lib import git
from lib import github

BINARY_EXTENSIONS = (".AppImage", ".apk", ".dmg", ".exe", ".flatpak")


@dataclass
class Config:
    upload: bool
    tag: str


def parse_args() -> Config:
    parser = argparse.ArgumentParser(description="""
    Download the binaries from the GitHub release, sign them, and upload
    the signatures to the GitHub release.
    """)
    parser.add_argument(
        "--upload",
        action=argparse.BooleanOptionalAction,
        help="Upload signatures to GitHub (disabling this is a dryrun)",
        default=True,
    )
    parser.add_argument(
        "--tag",
        help="Tag to create signatures for",
        default=git.current_tag(),
    )
    return Config(**vars(parser.parse_args()))


def needs_signing(name: str, asset_names: list[str]) -> bool:
    return (os.path.splitext(name)[1] in BINARY_EXTENSIONS
            and name + ".asc" not in asset_names)


def sign_binary(binary: str, tmpdir: str) -> None:
    print(f"Signing {binary}")
    subprocess.run(  # nosec
        [
            "gpg",
            "--armor",
            "--detach-sign",
            os.path.join(tmpdir, binary),
        ],
        check=True,
    )


def upload_signature(tag: str, tmpdir: str, binary: str) -> None:
    release_id = github.release_id(tag)
    print(f"Uploading signature for {binary}")
    with open(os.path.join(tmpdir, f"{binary}.asc"), "rb") as f:
        github.upload_asset(tag, f"{binary}.asc", "application/pgp-signature",
                            f)


def download_and_sign_binaries(config: Config, tmpdir: str) -> None:
    assets = github.release_assets(config.tag)
    asset_names = [asset.name for asset in assets]
    for asset in assets:
        if needs_signing(asset.name, asset_names):
            with open(os.path.join(tmpdir, asset.name), "wb") as f:
                print(f"Downloading {asset.name}")
                f.write(github.download_asset(asset.id))
            sign_binary(asset.name, tmpdir)
            if config.upload:
                upload_signature(config.tag, tmpdir, asset.name)


def main(config: Config) -> None:
    with tempfile.TemporaryDirectory() as tmpdir:
        download_and_sign_binaries(config, tmpdir)


if __name__ == "__main__":
    main(parse_args())
