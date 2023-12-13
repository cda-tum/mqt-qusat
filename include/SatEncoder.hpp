#pragma once

#include "CircuitOptimizer.hpp"
#include "QuantumComputation.hpp"
#include "Statistics.hpp"

#include <chrono>
#include <iostream>
#include <locale>
#include <nlohmann/json.hpp>
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

  SatEncoder::CircuitRepresentation
  preprocessCircuit(const qc::DAG& dag, const std::vector<std::string>& inputs);

  void constructSatInstance(
      const SatEncoder::CircuitRepresentation& circuitRepresentation,
      z3::solver& solver); // construct z3 instance. Assumes prepocessCircuit()
                           // has been run before.
  void constructMiterInstance(
      const SatEncoder::CircuitRepresentation& circuitOneRepresentation,
      const SatEncoder::CircuitRepresentation& circuitTwoRepresentation,
      z3::solver& solver); // assumes preprocess circuit has been run before

  bool isSatisfiable(z3::solver& solver);

  Statistics  stats;
  std::size_t nrOfInputGenerators = 0U;
  std::size_t uniqueGenCnt        = 0U;
};
