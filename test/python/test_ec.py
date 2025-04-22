# Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
# Copyright (c) 2025 Munich Quantum Software Company GmbH
# All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Licensed under the MIT License

"""Test module for MQT QuSAT."""

from __future__ import annotations

from qiskit.circuit import QuantumCircuit

from mqt.core import load
from mqt.core.ir import QuantumComputation
from mqt.qusat import check_equivalence


def test_equivalence() -> None:
    """A simple equivalence test."""
    qc1 = QuantumComputation(2)
    qc1.cx(0, 1)

    qc2 = QuantumComputation(2)
    qc2.h(0)
    qc2.h(1)
    qc2.cx(1, 0)
    qc2.h(1)
    qc2.h(0)

    result = check_equivalence(qc1, qc2)
    assert result["equivalent"]


def test_non_equivalence() -> None:
    """A simple non-equivalence test."""
    qc1 = QuantumComputation(2)
    qc1.cx(0, 1)

    qc2 = QuantumComputation(2)
    qc2.h(0)
    qc2.h(1)
    qc2.cx(0, 1)
    qc2.h(1)
    qc2.h(0)

    result = check_equivalence(qc1, qc2)
    assert not result["equivalent"]


def test_equivalence_qiskit() -> None:
    """A simple equivalence test with Qiskit circuits."""
    qc1 = QuantumCircuit(2)
    qc1.cx(0, 1)

    qc2 = QuantumCircuit(2)
    qc2.h(range(2))
    qc2.cx(1, 0)
    qc2.h(range(2))

    result = check_equivalence(load(qc1), load(qc2))
    assert result["equivalent"]
