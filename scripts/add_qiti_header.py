#!/usr/bin/env python3
import sys
from pathlib import Path
from datetime import date

HEADER_TEMPLATE = '''
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     {filename}
 *
 * @author   Adam Shield
 * @date     {date}
 *
 * @copyright (c) {year} Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/
'''

def generate_header(path: Path) -> str:
    today = date.today().isoformat()
    year = today.split('-')[0]
    return HEADER_TEMPLATE.format(
        filename=path.name,
        date=today,
        year=year
    )

def has_header(text: str) -> bool:
    return 'Qiti — C++ Profiling Library' in text.splitlines()[0]

def process_file(path: Path):
    try:
        content = path.read_text(encoding='utf-8')
    except Exception as e:
        print(f"Error reading {path}: {e}")
        return
    if has_header(content):
        print(f"Skipping (already has header): {path}")
        return
    header = generate_header(path)
    try:
        path.write_text(header + content, encoding='utf-8')
        print(f"Added header to: {path}")
    except Exception as e:
        print(f"Error writing {path}: {e}")


def main(target: Path):
    if target.is_file():
        if target.suffix in ('.cpp', '.hpp'):
            process_file(target)
        else:
            print(f"Skipped (not a .cpp or .hpp): {target}")
    elif target.is_dir():
        for ext in ('*.cpp', '*.hpp'):
            for file_path in target.rglob(ext):
                process_file(file_path)
    else:
        print(f"Path not found: {target}")

if __name__ == '__main__':
    if len(sys.argv) > 1:
        root = Path(sys.argv[1])
    else:
        root = Path('.')
    main(root)
