/*
 * This file is part of MQT QuSAT library which is released under the MIT
 * license. See file README.md or go to
 * https://github.com/lucasberent/qsatencoder for more information.
 */

#include "SatEncoder.hpp"
#include "python/qiskit/QuantumCircuit.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;

void importQuantumComputation(qc::QuantumComputation& qc,
                              const py::object&       circ) {
  if (py::isinstance<py::str>(circ)) {
    auto&& file2 = circ.cast<std::string>();
    qc.import(file2);
  } else {
    qc::qiskit::QuantumCircuit::import(qc, circ);
  }
}

py::dict checkEquivalence(const py::object& circ1, const py::object& circ2,
                          const std::vector<std::string>& inputs = {}) {
  qc::QuantumComputation qc1{}, qc2{};
  py::dict               results{};
  try {
    importQuantumComputation(qc1, circ1);
    importQuantumComputation(qc2, circ2);
  } catch (std::exception const& e) {
    py::print("Could not import circuitt: ", e.what());
    return {};
  }

  SatEncoder encoder{};
  try {
    results["equivalent"] = encoder.testEqual(qc1, qc2, inputs);
  } catch (std::exception const& e) {
    py::print("Could not check equivalence: ", e.what());
    return {};
  }
  results["statistics"] = encoder.getStats();
  return results;
}

PYBIND11_MODULE(pyqusat, m) {
  m.doc() =
      "Python interface for the MQT QuSAT quantum circuit satisfiability tool";

  m.def("check_equivalence", &checkEquivalence,
        "Check the equivalence of two clifford circuits for the given inputs."
        "If no inputs are given, the all zero state is used as input.",
        "circ1"_a, "circ2"_a, "inputs"_a = std::vector<std::string>());
}
