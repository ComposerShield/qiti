import json
import subprocess
from pathlib import Path

def extract_function_signatures(obj_path):
    """
    Extract function linkage names, return types, and parameter types
    from a Mach-O object file's DWARF debug info (JSON via dwarfdump).
    """
    # Run dwarfdump to get JSON of debug info
    proc = subprocess.run(
        ["xcrun", "dwarfdump", "--debug-info", "--json", str(obj_path)],
        stdout=subprocess.PIPE,
        check=True
    )
    dwarfd = json.loads(proc.stdout)

    signatures = []
    for cu in dwarfd.get("CU", []):
        for die in cu.get("abbrev", []):
            if die.get("tag") != "DW_TAG_subprogram":
                continue

            # Location filter: only functions from "Source/" files
            file_attr = die.get("attributes", {}).get("DW_AT_decl_file", {})
            if "Source/" not in file_attr.get("value", ""):
                continue

            # Linkage name (mangled)
            linkage = die.get("attributes", {}).get("DW_AT_linkage_name", {})
            mname = linkage.get("value")
            if not mname:
                continue

            # Return type DIE reference
            ret_attr = die.get("attributes", {}).get("DW_AT_type", {})
            ret_die_ref = ret_attr.get("value")

            # Collect parameter type DIE references
            param_refs = []
            for child in die.get("children", []):
                if child.get("tag") == "DW_TAG_formal_parameter":
                    p_attr = child.get("attributes", {}).get("DW_AT_type", {})
                    if p_attr.get("value"):
                        param_refs.append(p_attr["value"])

            signatures.append({
                "mangled": mname,
                "ret_die": ret_die_ref,
                "param_dies": param_refs,
                "die_offset": die.get("offset")
            })
    return signatures

# Example usage: scan all .o files under Source/
source_dir = Path("build/ATCore_Project.build/Debug/Objects-normal/x86_64")
all_signatures = []
for obj in source_dir.glob("*.o"):
    sigs = extract_function_signatures(obj)
    all_signatures.extend(sigs)

# Display the collected signatures
import pandas as pd
df = pd.DataFrame(all_signatures)
import ace_tools as tools; tools.display_dataframe_to_user("Extracted DWARF Signatures", df)