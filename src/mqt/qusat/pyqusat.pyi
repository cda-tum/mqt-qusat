# Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
# Copyright (c) 2025 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

from typing import Any

from mqt.core.ir import QuantumComputation

def check_equivalence(
    circ1: QuantumComputation,
    circ2: QuantumComputation,
) -> dict[str, Any]: ...
def generate_dimacs(
    circ: QuantumComputation,
) -> str: ...
