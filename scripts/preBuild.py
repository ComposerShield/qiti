#!/usr/bin/env python3
import subprocess
import re
import sys
from pathlib import Path

# Only include functions from your real source tree
SOURCE_PREFIX = "/Users/Shared/Jenkins/antares/antdev/auto-tune-core/Source"

def extract_function_signatures(obj_path):
    """
    Returns a set of strings like:
      <return_type> <mangled_symbol>(<param1_type>, <param2_type>, …)
    for every DW_TAG_subprogram in the .o that comes from SOURCE_PREFIX.
    """
    # 1) run dwarfdump
    proc = subprocess.run(
        ["xcrun", "dwarfdump", "--debug-info", str(obj_path)],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        check=True, text=True
    )
    dump = proc.stdout

    functions = set()

    # 2) split at each function DIE
    segments = re.split(r'(?=0x[0-9a-f]+:\s+DW_TAG_subprogram)', dump)

    for seg in segments[1:]:
        # a) filter by source file path
        m_file = re.search(r'DW_AT_decl_file\s+\("([^"]+)"\)', seg)
        if not m_file or not m_file.group(1).startswith(SOURCE_PREFIX):
            continue

        # b) grab the true mangled name
        m_link = re.search(r'DW_AT_linkage_name\s+\("([^"]+)"\)', seg)
        if not m_link:
            # no linkage_name? skip (unlikely for C++ functions)
            continue
        mangled = m_link.group(1)

        # c) collect all the DW_AT_type entries in this segment:
        #    first = return type, rest = parameters
        #    this pulls the *human* type strings, e.g. "int", "float*", etc.
        type_matches = re.findall(r'DW_AT_type\s+\([^"]*"([^"]+)"\)', seg)
        if type_matches:
            ret = type_matches[0]
            params = type_matches[1:]
        else:
            ret = "void"
            params = []

        sig = f"{ret} {mangled}({', '.join(params)})"
        functions.add(sig)

    return functions

def main():
    source_dir = Path(
        "/Users/Shared/Jenkins/antares/antdev/auto-tune-core/"
        "build/ATCore_Project.build/Debug/Objects-normal/x86_64"
    )

    all_signatures = set()

    for obj in source_dir.glob("*.o"):
        try:
            all_signatures |= extract_function_signatures(obj)
        except subprocess.CalledProcessError as e:
            print(f"Warning: dwarfdump failed on {obj.name}: {e}", file=sys.stderr)

    output_path = "function_signatures.txt"
    with open(output_path, "w") as f:
        for sig in sorted(all_signatures):
            f.write(sig + "\n")

    print(f"Wrote {len(all_signatures)} functions to {output_path}")

if __name__ == "__main__":
    main()