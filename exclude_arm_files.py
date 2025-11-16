Import("env")
import os

# Exclude ARM-specific assembly files from LVGL build
def exclude_arm_sources(node):
    """Exclude ARM Helium and NEON assembly files"""
    return None if any(x in node.get_path() for x in [
        "helium",
        "neon", 
        ".S"
    ]) else node

env.AddBuildMiddleware(exclude_arm_sources, "*")

print("ARM-specific files excluded from build")
