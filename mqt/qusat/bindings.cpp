/*
 * This file is part of MQT QuSAT library which is released under the MIT
 * license. See file README.md or go to
 * https://github.com/lucasberent/qsatencoder for more information.
 */

#include "SatEncoder.hpp"
#include "pybind11/pybind11.h"
#include "python/qiskit/QuantumCircuit.hpp"

#include <pybind11/stl.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

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
                          const std::vector<std::string>& inputs) {
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

py::dict checkEquivalence(const py::object& circ1, const py::object& circ2) {
  return checkEquivalence(circ1, circ2, {});
}

PYBIND11_MODULE(pyqusat, m) {
  m.doc() =
      "Python interface for the MQT QuSAT quantum circuit satisfiability tool";
  py::class_<Statistics>(m, "Statistics", "Statistics of the SAT solver")
      .def_readwrite("gates", &Statistics::nrOfGates,
                     R"pbdoc(Number of Gates)pbdoc")
      .def_readwrite("n_qubits", &Statistics::nrOfQubits,
                     R"pbdoc(Number of Qubits)pbdoc")
      .def_readwrite("n_sat_variables", &Statistics::nrOfSatVars,
                     R"pbdoc(Number of SAT variables)pbdoc")
      .def_readwrite("n_generators", &Statistics::nrOfGenerators,
                     R"pbdoc(Number of Generators)pbdoc")
      .def_readwrite("n_functional_constraints",
                     &Statistics::nrOfFunctionalConstr,
                     R"pbdoc(Number of Functional Constraints)pbdoc")
      .def_readwrite("circuit_depth", &Statistics::circuitDepth,
                     R"pbdoc(Number of Depth)pbdoc")
      .def_readwrite("z3_statistics", &Statistics::z3StatsMap,
                     R"pbdoc(Additional Statistics as reported by Z3)pbdoc")
      .def_readwrite("n_input_states", &Statistics::nrOfDiffInputStates,
                     R"pbdoc(Number of Input States)pbdoc")
      .def_readwrite("equivalent", &Statistics::equal,
                     R"pbdoc(If the two circuits are equivalent)pbdoc")
      .def_readwrite("satisfiable", &Statistics::satisfiable,
                     R"pbdoc(If the SAT instance is satisfiable)pbdoc")
      .def_readwrite("preprocessing_time", &Statistics::preprocTime,
                     R"pbdoc(Preprocessing time (ms))pbdoc")
      .def_readwrite("solving_time", &Statistics::solvingTime,
                     R"pbdoc(SAT solving time (ms))pbdoc")
      .def_readwrite("sat_construction_time", &Statistics::satConstructionTime,
                     R"pbdoc(SAT construction time (ms))pbdoc")
      .def("__repr__", &Statistics::toString);

  m.def("checkEquivalence",
        py::overload_cast<const py::object&, const py::object&,
                          const std::vector<std::string>&>(&checkEquivalence),
        "check the equivalence of two clifford circuits for the given inputs",
        "circ1"_a, "circ2"_a, "inputs"_a);
  m.def("checkEquivalence",
        py::overload_cast<const py::object&, const py::object&>(
            &checkEquivalence),
        "check the equivalence of two clifford circuits for the all zero state "
        "as single input");

#ifdef VERSION_INFO
  m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
  m.attr("__version__") = "dev";
#endif
}
