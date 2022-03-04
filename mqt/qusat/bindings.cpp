/*
* This file is part of MQT QuSAT library which is released under the MIT license.
* See file README.md or go to https://github.com/lucasberent/qsatencoder for more information.
*/

#include "SatEncoder.hpp"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind11_json/pybind11_json.hpp"
#include "qiskit/QuantumCircuit.hpp"

namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;

bool checkEquivalent(const py::object&              circ1,
                     const py::object&              circ2,
                     const std::vector<std::string>& inputs) {
    qc::QuantumComputation qc1{};
    try {
        if (py::isinstance<py::str>(circ1)) {
            auto&& file1 = circ1.cast<std::string>();
            qc1.import(file1);
        } else {
            qc::qiskit::QuantumCircuit::import(qc1, circ1);
        }
    } catch (std::exception const& e) {
        py::print("Could not import first circuit: ", e.what());
        return false;
    }

    qc::QuantumComputation qc2{};
    try {
        if (py::isinstance<py::str>(circ2)) {
            auto&& file2 = circ2.cast<std::string>();
            qc2.import(file2);
        } else {
            qc::qiskit::QuantumCircuit::import(qc2, circ2);
        }
    } catch (std::exception const& e) {
        py::print("Could not import second circuit: ", e.what());
        return false;
    }
    SatEncoder encoder{};
    try {
        return encoder.testEqual(qc1, qc2, inputs);
    } catch (std::exception const& e) {
        py::print("Could not check equivalence: ", e.what());
        return false;
    }
}

PYBIND11_MODULE(pyqusat, m) {
    m.doc() = "Python interface for the MQT QuSAT quantum circuit satisfiability tool";
    m.def("checkEquivalent", &checkEquivalent, "check the equivalence of two clifford circuits for the given inputs",
          "circ1"_a, "circ2"_a, "inputs"_a);

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}