/*
 * Copyright (c) 2023 - 2025 Chair for Design Automation, TUM
 * Copyright (c) 2025 Munich Quantum Software Company GmbH
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#pragma once

#include "Statistics.hpp"
#include "circuit_optimizer/CircuitOptimizer.hpp"
#include "ir/QuantumComputation.hpp"

#include <cstddef>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <z3++.h>

using json = nlohmann::json;

class SatEncoder {
public:
  /**
   * Takes two Clifford circuits, constructs SAT instance and checks if there is
   * an assignment that leads to outputs that differ.
   * @param circuit first circuit
   * @param circuitTwo second circuit
   * @param inputs input states to consider. In stabilizer representation, e.g.
   * ZZ == |00>. If empty all-zero state is assumed.
   * @return true if the circuits are equivalent (for given inputs)
   */
  bool testEqual(qc::QuantumComputation&         circuit,
                 qc::QuantumComputation&         circuitTwo,
                 const std::vector<std::string>& inputs);

  /**
   * Takes two Clifford circuits, constructs SAT instance and checks if there is
   * an assignment that leads to outputs that differ with all zero state as
   * single input state.
   * @param circuit first circuit
   * @param circuitTwo second circuit
   * @return true if the circuits are equivalent (for all zero state input)
   */
  bool testEqual(qc::QuantumComputation& circuit,
                 qc::QuantumComputation& circuitTwo);

  /**
   * Constructs SAT instance for input circuit and checks satisfiability for
   * given inputs
   * @param circuitOne circuit to construct SAT instance for
   * @param inputs input states to consider. In stabilizer representation, e.g.,
   * ZZ == |00>. If empty all-zero state is assumed.
   */
  bool checkSatisfiability(qc::QuantumComputation&         circuitOne,
                           const std::vector<std::string>& inputs);

  /**
   * Constructs SAT instance for input circuit and checks satisfiability with
   * all zero state as single input state
   * @param circuitOne circuit to construct SAT instance for
   */
  bool checkSatisfiability(qc::QuantumComputation& circuitOne);

  /**
   * Output the DIMACS CNF representation from Z3 of the given circuit.
   * @param circuit circuit to construct SAT instance for
   * @return The DIMACS CNF representation of circuit
   */
  std::string generateDIMACS(qc::QuantumComputation& circuit);

  [[nodiscard]] json              to_json() const { return stats.to_json(); }
  [[nodiscard]] const Statistics& getStats() const;

private:
  struct QState {
    unsigned long                  n;
    std::vector<std::vector<bool>> x;
    std::vector<std::vector<bool>> z;
    std::vector<int>               r;
    std::size_t                    prevGenId;

    [[nodiscard]] std::vector<std::vector<bool>> getLevelGenerator() const;
    void applyCNOT(unsigned long control, unsigned long target);
    void applyH(unsigned long target);
    void applyS(unsigned long target);
  };

  class CircuitRepresentation {
  public:
    std::vector<std::map<std::size_t, std::size_t>>
        generatorMappings; // list of generatorId <> generatorId maps. One map
                           // per level
    std::map<std::size_t, std::vector<std::vector<bool>>>
        idGeneratorMap; // id <> generator map
  };

  std::map<std::vector<std::vector<bool>>, std::size_t>
      generators; // generator <> id map for reverse lookup

  static QState initializeState(unsigned long      nrOfInputs,
                                const std::string& input);

  static bool isClifford(const qc::QuantumComputation& qc);

  CircuitRepresentation
  preprocessCircuit(const qc::CircuitOptimizer::DAG& dag,
                    const std::vector<std::string>&  inputs);

  void constructSatInstance(
      const CircuitRepresentation& circuitRepresentation,
      z3::solver& solver); // construct z3 instance. Assumes prepocessCircuit()
                           // has been run before.
  void constructMiterInstance(
      const CircuitRepresentation& circuitOneRepresentation,
      const CircuitRepresentation& circuitTwoRepresentation,
      z3::solver& solver); // assumes preprocess circuit has been run before

  bool isSatisfiable(z3::solver& solver);

  Statistics  stats;
  std::size_t nrOfInputGenerators = 0U;
  std::size_t uniqueGenCnt        = 0U;
};
