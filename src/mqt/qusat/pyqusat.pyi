import os
from typing import Any

from qiskit import QuantumCircuit

def check_equivalence(
    circ1: str | os.PathLike[str] | QuantumCircuit,
    circ2: str | os.PathLike[str] | QuantumCircuit,
) -> dict[str, Any]: ...
