"""Run portable exact-helper cleanup checks; requires clang++ and Python 3."""
import pathlib, subprocess, tempfile
base = pathlib.Path(__file__).resolve().parent
with tempfile.TemporaryDirectory(prefix="electron-scratch-") as temp:
    source = pathlib.Path(temp) / "src/libANGLE/renderer/gl/TextureGL.cpp"
    source.parent.mkdir(parents=True)
    source.write_bytes((base / "scratch-exact-function.inc").read_bytes())
    subprocess.run(["git", "init", "-q", temp], check=True)
    subprocess.run(["git", "apply", str(base / "scratch-repair.patch")], cwd=temp, check=True)
    assert source.read_bytes() == (base / "scratch-fixed-function.inc").read_bytes()
    for assertions in (False, True):
        for repaired in (False, True):
            binary = str(pathlib.Path(temp) / "check")
            command = ["clang++", "-std=c++20", "-include", "initializer_list"]
            if assertions:
                command.append("-DANGLE_ENABLE_ASSERTS")
            if repaired:
                command.append('-DFUNCTION_FILE="scratch-fixed-function.inc"')
            subprocess.run(command + [str(base / "scratch-cleanup-check.cc"), "-o", binary], check=True)
            result = subprocess.run([binary])
            assert result.returncode == (0 if repaired else 1), (assertions, repaired, result.returncode)
print("PASS: baseline fails and repair passes all 7 release / 9 assertion cases")
