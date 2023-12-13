"""MQT QuSAT library.

This file is part of the MQT QuSAT library released under the MIT license.
See README.md or go to https://github.com/cda-tum/mqt-qusat for more information.
"""

from __future__ import annotations

import os
import sys
from pathlib import Path

if sys.platform == "win32" and sys.version_info > (3, 8, 0) and "Z3_ROOT" in os.environ:
    lib_path = Path(os.environ["Z3_ROOT"]) / "lib"
    if lib_path.exists():
        os.add_dll_directory(str(lib_path))
    bin_path = Path(os.environ["Z3_ROOT"]) / "bin"
    if bin_path.exists():
        os.add_dll_directory(str(bin_path))

from .pyqusat import check_equivalence

__all__ = ["check_equivalence"]
