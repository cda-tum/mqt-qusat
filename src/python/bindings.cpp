/*
 * Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
 * Copyright (c) 2025 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#include "SatEncoder.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11_json/pybind11_json.hpp> // IWYU pragma: keep

namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;

nl::basic_json<> checkEquivalence(qc::QuantumComputation&         qc1,
                                  qc::QuantumComputation&         qc2,
                                  const std::vector<std::string>& inputs = {}) {
  nl::basic_json results{};
  SatEncoder     encoder{};
  try {
    results["equivalent"] = encoder.testEqual(qc1, qc2, inputs);
  } catch (std::exception const& e) {
    py::print("Could not check equivalence: ", e.what());
    return {};
  }
  results["statistics"] = encoder.getStats().to_json();
  return results;
}

std::string printDIMACS(qc::QuantumComputation& qc) {
  SatEncoder encoder{};
  return encoder.generateDIMACS(qc);
}

PYBIND11_MODULE(pyqusat, m) {
  m.doc() =
      "Python interface for the MQT QuSAT quantum circuit satisfiability tool";

  m.def("check_equivalence", &checkEquivalence,
        "Check the equivalence of two clifford circuits for the given inputs."
        "If no inputs are given, the all zero state is used as input.",
        "circ1"_a, "circ2"_a, "inputs"_a = std::vector<std::string>());

  m.def("generate_dimacs", &printDIMACS,
        "Output the DIMACS CNF representation from Z3 of the given circuit.",
        "circ"_a);
}
