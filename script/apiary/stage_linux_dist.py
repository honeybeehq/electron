#!/usr/bin/env python3
"""Validate and stage a Linux fork dist without publishing or replacing assets."""

import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import stat
import struct
import zipfile


MACHINES = {"x64": 62, "arm64": 183}
MANIFESTS = Path(__file__).resolve().parents[1] / "zip_manifests"


def validate_archive(archive, version, arch):
    """Check the upstream Linux layout, executable target, and runtime version."""
    if not re.fullmatch(r"[0-9]+\.[0-9]+\.[0-9]+", version):
        raise ValueError("version must be the npm Electron version, without the fork suffix")
    if arch not in MACHINES:
        raise ValueError(f"unsupported Linux architecture: {arch}")
    expected = set((MANIFESTS / f"dist_zip.linux.{arch}.manifest").read_text().splitlines())
    with zipfile.ZipFile(archive) as dist:
        names = dist.namelist()
        if len(names) != len(set(names)):
            raise ValueError("archive contains duplicate entries")
        files = {entry.filename for entry in dist.infolist() if not entry.is_dir()}
        if files != expected:
            raise ValueError(f"Linux dist layout differs: missing={sorted(expected - files)}, extra={sorted(files - expected)}")
        if dist.read("version").decode().strip().removeprefix("v") != version:
            raise ValueError("archive version does not match the requested Electron version")
        for name in ("electron", "chrome-sandbox", "chrome_crashpad_handler"):
            entry = dist.getinfo(name)
            mode = entry.external_attr >> 16
            if not stat.S_ISREG(mode) or not mode & 0o111:
                raise ValueError(f"{name} must be a regular executable file")
            with dist.open(name) as binary:
                header = binary.read(20)
            if len(header) < 20 or header[:6] != b"\x7fELF\x02\x01":
                raise ValueError(f"{name} is not a 64-bit little-endian ELF binary")
            if struct.unpack_from("<H", header, 18)[0] != MACHINES[arch]:
                raise ValueError(f"{name} architecture does not match {arch}")
        if dist.testzip() is not None:
            raise ValueError("archive failed its CRC integrity check")


def stage_archive(archive, output, version, arch):
    """Create a new staging directory; existing release bytes are never overwritten."""
    archive = Path(archive)
    output = Path(output)
    validate_archive(archive, version, arch)
    output.mkdir(parents=True, exist_ok=False)
    asset = output / f"electron-v{version}-linux-{arch}.zip"
    with archive.open("rb") as source, asset.open("xb") as target:
        shutil.copyfileobj(source, target)
    with asset.open("rb") as staged:
        digest = hashlib.file_digest(staged, "sha256").hexdigest()
    # Platform fragments are deliberately not called SHASUMS256.txt: release
    # assembly must include the macOS entries instead of clobbering them.
    checksum = output / f"SHASUMS256-linux-{arch}.txt"
    checksum.write_text(f"{digest} *{asset.name}\n", encoding="utf-8")
    return {"asset": str(asset), "checksum": str(checksum), "sha256": digest}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path, help="electron:electron_dist_zip output")
    parser.add_argument("--output", type=Path, required=True, help="new staging directory")
    parser.add_argument("--version", required=True, help="npm version, e.g. 43.4.1")
    parser.add_argument("--arch", choices=sorted(MACHINES), required=True)
    args = parser.parse_args()
    try:
        print(json.dumps(stage_archive(args.archive, args.output, args.version, args.arch)))
    except (OSError, ValueError, zipfile.BadZipFile) as error:
        parser.exit(1, f"stage-linux-dist: {error}\n")


if __name__ == "__main__":
    main()
