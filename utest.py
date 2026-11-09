#!/usr/bin/env python3
"""Runs the unit test executables (utcommon and utpar) of a built project.

Usage: python3 utest.py
Build first, e.g.: cmake --preset default && cmake --build --preset gcc-release
"""

import os
import subprocess
import sys
from pathlib import Path

# Build directories produced by the presets, in order of preference.
BUILD_DIRS = [
    "out/build/default/{name}/Release/{name}",
    "out/build/default/{name}/Debug/{name}",
    "out/build/clang-tidy/{name}/Release/{name}",
    "out/build/clang-tidy/{name}/Debug/{name}",
]


def find_executable(name):
    """Returns the path of the test executable, or None if not built."""
    for pattern in BUILD_DIRS:
        for ext in ("", ".exe"):
            path = Path(pattern.format(name=name) + ext)
            if path.exists():
                return path
    return None


def run_test(executable):
    """Runs a test executable and returns its exit code."""
    try:
        return subprocess.run([str(executable)], timeout=120,
                              env=os.environ.copy()).returncode
    except subprocess.TimeoutExpired:
        print(f"TIMEOUT: {executable}")
        return 1


def main():
    executables = {name: find_executable(name) for name in ("utcommon", "utpar")}
    missing = [name for name, path in executables.items() if path is None]
    if missing:
        print("ERROR: test executables not found: " + ", ".join(missing))
        print("Build the project first (see README).")
        return 1

    exit_code = 0
    for name, path in executables.items():
        print(f"== {name}: {path}")
        if run_test(path) != 0:
            exit_code = 1
    return exit_code


if __name__ == "__main__":
    sys.exit(main())
