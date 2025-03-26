from typing import Any

from mqt.core.ir import QuantumComputation

def check_equivalence(
    circ1: QuantumComputation,
    circ2: QuantumComputation,
) -> dict[str, Any]: ...
