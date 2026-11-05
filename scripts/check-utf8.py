#!/usr/bin/env python3
"""Fail if any text file in the repo cannot be decoded as UTF-8."""

from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

SKIP_DIR_NAMES = {
    ".git",
    ".venv",
    "venv",
    "node_modules",
    "__pycache__",
    ".pio",
    "dist",
    "build",
    ".tox",
}

# Binary / non-text extensions we never decode as UTF-8.
SKIP_SUFFIXES = {
    ".png",
    ".jpg",
    ".jpeg",
    ".gif",
    ".webp",
    ".ico",
    ".pdf",
    ".zip",
    ".gz",
    ".tgz",
    ".7z",
    ".bin",
    ".elf",
    ".o",
    ".a",
    ".so",
    ".dll",
    ".exe",
    ".pyc",
    ".pyo",
    ".whl",
    ".pem",
    ".der",
    ".p12",
    ".ttf",
    ".otf",
    ".woff",
    ".woff2",
    ".log",
    ".pcap",
    ".sqlite",
    ".db",
}


def is_skipped(path: Path) -> bool:
    if any(part in SKIP_DIR_NAMES for part in path.parts):
        return True
    if path.suffix.lower() in SKIP_SUFFIXES:
        return True
    return False


def main() -> int:
    bad: list[str] = []
    for path in sorted(ROOT.rglob("*")):
        if not path.is_file() or is_skipped(path):
            continue
        raw = path.read_bytes()
        try:
            raw.decode("utf-8")
        except UnicodeDecodeError as exc:
            rel = path.relative_to(ROOT).as_posix()
            bad.append(f"{rel}: {exc}")

    if bad:
        print("UTF-8 check failed:", file=sys.stderr)
        for line in bad:
            print(f"  {line}", file=sys.stderr)
        return 1

    print("UTF-8 check passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
