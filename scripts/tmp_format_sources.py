#!/usr/bin/env python3

from pathlib import Path
import sys

done_marker = ".formatted"

def traversal_for(d):
    """
    Returns a list of the directories in `d` (including `d`) that have not
    yet been completely formatted (e.g. accepted by the developer).
    """
    if (d / done_marker).exists():
        return []  # Everything's processed

    subdirs = sorted([p for p in d.iterdir() if p.is_dir() and "resources" not in p.name and "shims" not in p.name])
    rv = []
    for subdir in subdirs:
        rv.extend(traversal_for(subdir))
    rv.append(d)
    return rv

def format_directory(d):
    """
    Formats all sources in `d` using .clang-format
    """
    print(d)

dirs = [Path(d) for d in [
    'hellooscar',
    'libopensimcreator',
    'libopynsim',
    'liboscar',
    'liboscar-demos',
    'opynsim_debugger',
    'osc',
    'oscar_demo_viewer',
    'src',
]]
dirs.sort()
traversals = []
for d in dirs:
    assert d.exists(), f"{d}: does not exist"
    d_traversals = traversal_for(d)
    traversals += d_traversals
if not traversals:
    print("Nothing left to do!")
    sys.exit(0)
print(traversals[0])

