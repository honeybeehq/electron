"""Packaging regressions use tiny ELF fixtures, never downloaded binaries."""

import hashlib
from pathlib import Path
import stat
import struct
import tempfile
import unittest
import zipfile

from stage_linux_dist import MANIFESTS, MACHINES, stage_archive, validate_archive


class LinuxDistTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def archive(self, arch="x64", version="43.4.1", missing=None, machine=None, executable=True):
        path = self.root / "dist.zip"
        names = (MANIFESTS / f"dist_zip.linux.{arch}.manifest").read_text().splitlines()
        with zipfile.ZipFile(path, "w") as dist:
            for name in names:
                if name == missing:
                    continue
                entry = zipfile.ZipInfo(name)
                entry.create_system = 3
                entry.external_attr = (stat.S_IFREG | 0o644) << 16
                data = b"fixture"
                if name == "version":
                    data = version.encode()
                elif name in ("electron", "chrome-sandbox", "chrome_crashpad_handler"):
                    data = b"\x7fELF\x02\x01" + bytes(12) + struct.pack("<H", machine or MACHINES[arch])
                    if executable:
                        entry.external_attr = (stat.S_IFREG | 0o755) << 16
                dist.writestr(entry, data)
        return path

    def test_stages_standard_assets_and_remote_checksum_for_each_arch(self):
        for arch in MACHINES:
            with self.subTest(arch=arch):
                archive = self.archive(arch=arch)
                result = stage_archive(archive, self.root / arch, "43.4.1", arch)
                asset = Path(result["asset"])
                self.assertEqual(asset.name, f"electron-v43.4.1-linux-{arch}.zip")
                self.assertEqual(asset.read_bytes(), archive.read_bytes())
                digest = hashlib.sha256(archive.read_bytes()).hexdigest()
                self.assertEqual(Path(result["checksum"]).read_text(), f"{digest} *{asset.name}\n")
                self.assertFalse((asset.parent / "SHASUMS256.txt").exists())

    def test_rejects_wrong_version_or_fork_suffix(self):
        archive = self.archive()
        for version in ("43.2.0", "43.4.1-apiary.4", "../43.4.1"):
            with self.subTest(version=version), self.assertRaises(ValueError):
                validate_archive(archive, version, "x64")

    def test_requires_complete_runtime_layout(self):
        for missing in ("electron", "libffmpeg.so", "chrome-sandbox", "locales/en-US.pak", "version"):
            with self.subTest(missing=missing), self.assertRaisesRegex(ValueError, "layout"):
                validate_archive(self.archive(missing=missing), "43.4.1", "x64")

    def test_checks_elf_architecture_not_build_host_name(self):
        with self.assertRaisesRegex(ValueError, "architecture"):
            validate_archive(self.archive(machine=MACHINES["arm64"]), "43.4.1", "x64")

    def test_requires_executable_permissions(self):
        with self.assertRaisesRegex(ValueError, "executable"):
            validate_archive(self.archive(executable=False), "43.4.1", "x64")

    def test_refuses_existing_staging_directory_without_overwriting(self):
        output = self.root / "release"
        output.mkdir()
        checksum = output / "SHASUMS256.txt"
        checksum.write_text("existing macOS checksum\n")
        with self.assertRaises(FileExistsError):
            stage_archive(self.archive(), output, "43.4.1", "x64")
        self.assertEqual(checksum.read_text(), "existing macOS checksum\n")
        self.assertEqual(list(output.iterdir()), [checksum])


if __name__ == "__main__":
    unittest.main()
