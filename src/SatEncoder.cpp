/*
 * Copyright (c) 2025 Chair for Design Automation, TUM
 * All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Licensed under the MIT License
 */

#include "SatEncoder.hpp"

#include "ir/QuantumComputation.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <z3++.h>

bool SatEncoder::testEqual(qc::QuantumComputation&         circuit,
                           qc::QuantumComputation&         circuitTwo,
                           const std::vector<std::string>& inputs) {
  if (!isClifford(circuit) || !isClifford(circuitTwo)) {
    std::cerr << "Circuits are not Clifford circuits" << std::endl;
    return false;
  }
  if (circuit.empty() || circuitTwo.empty()) {
    std::cerr << "Both circuits must be non-empty" << std::endl;
    return false;
  }
  stats.nrOfDiffInputStates = inputs.size();
  stats.nrOfQubits          = circuit.getNqubits();
  const auto dagOne         = qc::CircuitOptimizer::constructDAG(circuit);
  const auto dagTwo         = qc::CircuitOptimizer::constructDAG(circuitTwo);
  const CircuitRepresentation circOneRep = preprocessCircuit(dagOne, inputs);
  const CircuitRepresentation circTwoRep = preprocessCircuit(dagTwo, inputs);
  z3::context                 ctx{};
  z3::solver                  solver(ctx);
  constructMiterInstance(circOneRep, circTwoRep, solver);

  const bool equal = !isSatisfiable(solver);
  stats.equal      = equal;

  return equal;
}

bool SatEncoder::testEqual(qc::QuantumComputation& circuit,
                           qc::QuantumComputation& circuitTwo) {
  return testEqual(circuit, circuitTwo, {});
}

bool SatEncoder::checkSatisfiability(qc::QuantumComputation& circuitOne) {
  return checkSatisfiability(circuitOne, {});
}

bool SatEncoder::checkSatisfiability(qc::QuantumComputation&         circuitOne,
                                     const std::vector<std::string>& inputs) {
  if (!isClifford(circuitOne)) {
    std::cerr << "Circuit is not Clifford Circuit." << std::endl;
    return false;
  }
  stats.nrOfDiffInputStates = inputs.size();
  stats.nrOfQubits          = circuitOne.getNqubits();
  const auto  dag           = qc::CircuitOptimizer::constructDAG(circuitOne);
  const auto  circRep       = preprocessCircuit(dag, inputs);
  z3::context ctx{};
  z3::solver  solver(ctx);
  constructSatInstance(circRep, solver);

  stats.satisfiable = this->isSatisfiable(solver);
  return stats.satisfiable;
}

bool SatEncoder::isSatisfiable(z3::solver& solver) {
  stats.satisfiable = false;
  auto before       = std::chrono::high_resolution_clock::now();
  auto sat          = solver.check();
  auto after        = std::chrono::high_resolution_clock::now();
  auto z3SolvingDuration =
      std::chrono::duration_cast<std::chrono::milliseconds>(after - before)
          .count();
  stats.solvingTime = static_cast<std::size_t>(z3SolvingDuration);

  if (sat == z3::check_result::sat) {
    stats.satisfiable = true;
  }

  for (size_t i = 0; i < solver.statistics().size(); i++) {
    auto   key = solver.statistics().key(static_cast<unsigned>(i));
    double val;
    if (solver.statistics().is_double(static_cast<unsigned>(i))) {
      val = solver.statistics().double_value(static_cast<unsigned>(i));
    } else {
      val = solver.statistics().uint_value(static_cast<unsigned>(i));
    }
    stats.z3StatsMap.emplace(key, val);
  }
  return stats.satisfiable;
}

SatEncoder::CircuitRepresentation
SatEncoder::preprocessCircuit(const qc::CircuitOptimizer::DAG& dag,
                              const std::vector<std::string>&  inputs) {
  const auto            before     = std::chrono::high_resolution_clock::now();
  const std::size_t     inputSize  = dag.size();
  std::size_t           nrOfLevels = 0;
  std::size_t           nrOfOpsOnQubit = 0;
  std::vector<QState>   states;
  CircuitRepresentation representation;
  const auto            nrOfQubits = dag.size();

  // compute nr of levels of ckt = #generators needed per input state
  for (std::size_t i = 0U; i < inputSize; i++) {
    if (const auto tmp = dag.at(i).size(); tmp > nrOfLevels) {
      nrOfLevels = tmp;
    }
  }

  stats.circuitDepth =
      nrOfLevels > stats.circuitDepth ? nrOfLevels : stats.circuitDepth;
  representation.generatorMappings =
      std::vector<std::map<std::size_t, std::size_t>>(nrOfLevels);

  if (!inputs.empty()) {
    for (auto& input : inputs) {
      states.push_back(initializeState(nrOfQubits, input));
    }
  } else {
    states.push_back(initializeState(nrOfQubits, {}));
  }

  // store generators of input state
  for (auto& state : states) {
    auto        initLevelGenerator = state.getLevelGenerator();
    auto        inspair = generators.emplace(initLevelGenerator, uniqueGenCnt);
    std::size_t id;
    if (inspair.second) {
      id = uniqueGenCnt++;
    } else {
      id = generators.at(initLevelGenerator);
    }
    representation.idGeneratorMap.emplace(id, initLevelGenerator);
    state.prevGenId = id;
  }

  if (nrOfInputGenerators == 0) { // only in first pass
    nrOfInputGenerators = uniqueGenCnt;
  }

  for (std::size_t levelCnt = 0; levelCnt < nrOfLevels; levelCnt++) {
    for (std::size_t qubitCnt = 0U; qubitCnt < inputSize;
         qubitCnt++) { // operation of current level for each qubit
      nrOfOpsOnQubit = dag.at(qubitCnt).size();

      if (levelCnt < nrOfOpsOnQubit) {
        if (!dag.at(qubitCnt).empty() &&
            dag.at(qubitCnt).at(levelCnt) != nullptr) {
          stats.nrOfGates++;
          const auto gate = dag.at(qubitCnt).at(levelCnt)->get();
          const auto target =
              gate->getTargets().at(0U); // we assume we only have 1 target

          for (auto& currState : states) {
            if (gate->getType() == qc::OpType::H) {
              currState.applyH(target);
            } else if (gate->getType() == qc::OpType::S) {
              currState.applyS(target);
            } else if (gate->getType() == qc::OpType::Sdg) {
              currState.applyS(target); // Sdag == SSS
              currState.applyS(target);
              currState.applyS(target);
            } else if (gate->getType() == qc::OpType::Z) {
              currState.applyH(target);
              currState.applyS(target);
              currState.applyS(target);
              currState.applyH(target);
            } else if (gate->getType() == qc::OpType::X &&
                       !gate->isControlled()) {
              currState.applyH(target);
              currState.applyS(target);
              currState.applyS(target);
            } else if (gate->getType() == qc::OpType::Y) {
              currState.applyH(target);
              currState.applyS(target);
              currState.applyS(target);
              currState.applyS(target);
            } else if (gate->isControlled() &&
                       gate->getType() == qc::OpType::X) { // CNOT
              const auto control =
                  gate->getControls()
                      .begin()
                      ->qubit; // we assume we only have 1 control
              if (qubitCnt ==
                  control) { // CNOT is for control and target in DAG, only
                             // apply if current qubit is control
                currState.applyCNOT(control, target);
              }
            }
          }
        }
      }
    }
    for (auto& state : states) {
      auto        currLevelGen = state.getLevelGenerator();
      auto        inspair      = generators.emplace(currLevelGen, uniqueGenCnt);
      std::size_t id;
      if (inspair.second) {
        id = uniqueGenCnt;
        uniqueGenCnt++;
      } else {
        id = generators.at(currLevelGen);
      }
      representation.idGeneratorMap.emplace(id, currLevelGen);
      representation.generatorMappings.at(levelCnt).emplace(state.prevGenId,
                                                            id);
      state.prevGenId = id;
    }
  }
  auto after = std::chrono::high_resolution_clock::now();
  stats.preprocTime += static_cast<std::size_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(after - before)
          .count());
  return representation;
}

// construct z3 instance from preprocessing information
void SatEncoder::constructSatInstance(
    const CircuitRepresentation& circuitRepresentation, z3::solver& solver) {
  auto before = std::chrono::high_resolution_clock::now();
  // number of unique generators that need to be encoded
  const auto generatorCnt = generators.size();
  if (generatorCnt < 1) {
    std::cerr << "Zero generators computed" << std::endl;
    return;
  }
  stats.nrOfGenerators = generatorCnt;
  // bitwidth required to encode the generators
  auto bitwidth = static_cast<std::size_t>(std::ceil(std::log2(generatorCnt)));
  if (bitwidth < 1 && generatorCnt == 1) {
    bitwidth = 1;
  }
  // whether the number of generators is a power of two or not
  bool blockingConstraintsNeeded =
      std::log2(generatorCnt) < static_cast<double>(bitwidth);

  // z3 context used throughout this function
  auto& ctx = solver.ctx();

  const auto depth = circuitRepresentation.generatorMappings.size();

  std::vector<z3::expr> vars{};
  vars.reserve(depth + 1U);
  std::string bvName = "x^";

  for (std::size_t k = 0U; k <= depth; k++) {
    // create bitvector [x^k]_2 with respective bitwidth for each level k of ckt
    std::stringstream ss{};
    ss << bvName << k; //
    vars.emplace_back(
        ctx.bv_const(ss.str().c_str(), static_cast<unsigned>(bitwidth)));
    stats.nrOfSatVars++;
  }

  for (std::size_t i = 0U; i < depth; i++) {
    const auto layer = circuitRepresentation.generatorMappings.at(
        i); // generator<>generator map for level i
    for (const auto& [from, to] : layer) {
      const auto g1 =
          generators.at(circuitRepresentation.idGeneratorMap.at(from));
      const auto g2 =
          generators.at(circuitRepresentation.idGeneratorMap.at(to));

      // create [x^l]_2 = i => [x^l']_2 = k for each generator mapping
      const auto left = vars[i] == ctx.bv_val(static_cast<std::uint64_t>(g1),
                                              static_cast<unsigned>(bitwidth));
      const auto right =
          vars[i + 1U] == ctx.bv_val(static_cast<std::uint64_t>(g2),
                                     static_cast<unsigned>(bitwidth));
      const auto cons = implies(left, right);
      solver.add(cons);
      stats.nrOfFunctionalConstr++;
    }
  }

  if (blockingConstraintsNeeded) {
    for (const auto& var : vars) {
      const auto cons =
          ult(var, ctx.bv_val(static_cast<std::uint64_t>(generatorCnt),
                              static_cast<unsigned>(bitwidth)));
      solver.add(cons); // [x^l]_2 < m
    }
  }
  auto after                = std::chrono::high_resolution_clock::now();
  stats.satConstructionTime = static_cast<std::size_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(after - before)
          .count());
}

void SatEncoder::constructMiterInstance(const CircuitRepresentation& circOneRep,
                                        const CircuitRepresentation& circTwoRep,
                                        z3::solver&                  solver) {
  auto before = std::chrono::high_resolution_clock::now();
  // number of unique generators that need to be encoded
  const auto generatorCnt = generators.size();
  if (generatorCnt < 1) {
    std::cerr << "Zero generators computed" << std::endl;
    return;
  }
  stats.nrOfGenerators = generatorCnt;
  // bitwidth required to encode the generators
  auto bitwidth = static_cast<std::size_t>(std::ceil(std::log2(generatorCnt)));
  if (bitwidth < 1 && generatorCnt == 1) {
    bitwidth = 1;
  }

  // whether the number of generators is a power of two or not
  bool blockingConstraintsNeeded =
      std::log2(generatorCnt) < static_cast<double>(bitwidth);
  // z3 context used throughout this function
  auto& ctx = solver.ctx();

  /// encode first circuit
  const auto            depthOne = circOneRep.generatorMappings.size();
  std::vector<z3::expr> varsOne{};
  varsOne.reserve(depthOne + 1U);
  std::string bvName = "x^";

  for (std::size_t k = 0U; k <= depthOne; k++) {
    // create bitvector [x^k]_2 with respective bitwidth for each level k of ckt
    std::stringstream ss{};
    ss << bvName << k; //
    const auto tmp =
        ctx.bv_const(ss.str().c_str(), static_cast<unsigned>(bitwidth));
    varsOne.emplace_back(tmp);
    stats.nrOfSatVars++;
  }

  for (std::size_t i = 0U; i < depthOne; i++) {
    const auto layer = circOneRep.generatorMappings.at(
        i); // generator<>generator map for level i
    for (const auto& [from, to] : layer) {
      const auto g1 = generators.at(circOneRep.idGeneratorMap.at(from));
      const auto g2 = generators.at(circOneRep.idGeneratorMap.at(to));

      // create [x^l]_2 = i <=> [x^l']_2 = k for each generator mapping
      const auto left =
          varsOne[i] == ctx.bv_val(static_cast<std::uint64_t>(g1),
                                   static_cast<unsigned>(bitwidth));
      const auto right =
          varsOne[i + 1U] == ctx.bv_val(static_cast<std::uint64_t>(g2),
                                        static_cast<unsigned>(bitwidth));
      const auto cons = (left == right);
      solver.add(cons);
      stats.nrOfFunctionalConstr++;
    }
  }

  if (blockingConstraintsNeeded) {
    for (const auto& var : varsOne) {
      const auto cons =
          ult(var, ctx.bv_val(static_cast<std::uint64_t>(generatorCnt),
                              static_cast<unsigned>(bitwidth)));
      solver.add(cons); // [x^l]_2 < m
    }
  }
  /// encode second circuit
  auto                  depthTwo = circTwoRep.generatorMappings.size();
  std::vector<z3::expr> varsTwo{};
  varsOne.reserve(depthTwo + 1U);
  bvName = "x'^";

  for (std::size_t k = 0U; k <= depthTwo; k++) {
    // create bitvector [x^k]_2 with respective bitwidth for each level k of ckt
    std::stringstream ss{};
    ss << bvName << k; //
    const auto tmp =
        ctx.bv_const(ss.str().c_str(), static_cast<unsigned>(bitwidth));
    varsTwo.emplace_back(tmp);
    stats.nrOfSatVars++;
  }

  for (std::size_t i = 0U; i < depthTwo; i++) {
    const auto layer = circTwoRep.generatorMappings.at(
        i); // generator<>generator map for level i
    for (const auto& [from, to] : layer) {
      const auto g1 = generators.at(circTwoRep.idGeneratorMap.at(from));
      const auto g2 = generators.at(circTwoRep.idGeneratorMap.at(to));

      // create [x^l]_2 = i <=> [x^l']_2 = k for each generator mapping
      const auto left =
          varsTwo[i] == ctx.bv_val(static_cast<std::uint64_t>(g1),
                                   static_cast<unsigned>(bitwidth));
      const auto right =
          varsTwo[i + 1U] == ctx.bv_val(static_cast<std::uint64_t>(g2),
                                        static_cast<unsigned>(bitwidth));
      const auto cons = (left == right);
      solver.add(cons);
      stats.nrOfFunctionalConstr++;
    }
  }

  if (blockingConstraintsNeeded) {
    for (const auto& var : varsTwo) {
      const auto cons =
          ult(var, ctx.bv_val(static_cast<std::uint64_t>(generatorCnt),
                              static_cast<unsigned>(bitwidth)));
      solver.add(cons); // [x^l]_2 < m
    }
  }
  // create miter structure
  // if initial signals are the same, then the final signals have to be equal as
  // well
  const auto equalInputs    = varsOne.front() == varsTwo.front();
  const auto unequalOutputs = varsOne.back() != varsTwo.back();
  const auto nrOfInputs =
      ctx.bv_val(static_cast<std::uint64_t>(nrOfInputGenerators),
                 static_cast<unsigned>(bitwidth));
  const auto input1 = ult(varsOne.front(), nrOfInputs);
  const auto input2 = ult(varsTwo.front(), nrOfInputs);

  solver.add(equalInputs);
  solver.add(unequalOutputs);
  solver.add(input1);
  solver.add(input2);
  auto after                = std::chrono::high_resolution_clock::now();
  stats.satConstructionTime = static_cast<std::size_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(after - before)
          .count());
}

bool SatEncoder::isClifford(const qc::QuantumComputation& qc) {
  qc::OpType opType;
  for (const auto& op : qc) {
    opType = op->getType();
    if (opType != qc::OpType::H && opType != qc::OpType::S &&
        opType != qc::OpType::Sdg && opType != qc::OpType::X &&
        opType != qc::OpType::Z && opType != qc::OpType::Y &&
        opType != qc::OpType::I) {
      return false;
    }
  }
  return true;
}

std::vector<std::vector<bool>> SatEncoder::QState::getLevelGenerator() const {
  std::size_t                    size = (2U * n) + 1U;
  std::vector<std::vector<bool>> result{};

  for (std::size_t i = 0U; i < n; i++) {
    std::vector<bool> gen(size);
    for (std::size_t j = 0U; j < n; j++) {
      gen[j] = x.at(i).at(j);
    }
    for (std::size_t j = 0; j < n; j++) {
      gen[n + j] = z.at(i).at(j);
    }
    gen[n + n] = r.at(i) == 1;
    result.emplace_back(gen);
  }

  return result;
}

SatEncoder::QState SatEncoder::initializeState(unsigned long      nrOfQubits,
                                               const std::string& input) {
  QState result;
  result.n = nrOfQubits;
  result.x =
      std::vector<std::vector<bool>>(nrOfQubits, std::vector<bool>(nrOfQubits));
  result.z =
      std::vector<std::vector<bool>>(nrOfQubits, std::vector<bool>(nrOfQubits));
  result.r = std::vector<int>(nrOfQubits, 0);

  for (std::size_t i = 0U; i < nrOfQubits; i++) {
    result.z[i][i] = true; // initial 0..0 state corresponds to x matrix all
                           // zero and z matrix = Id_n
  }

  if (!input.empty()) { //
    for (std::size_t i = 0U; i < input.length(); i++) {
      switch (input[i]) {
      case 'Z': // stab by -Z = |1>
        result.applyH(i);
        result.applyS(i);
        result.applyS(i);
        result.applyH(i);
        break;
      case 'x': // stab by X = |+>
        result.applyH(i);
        break;
      case 'X': // stab by -X = |->
        result.applyH(i);
        result.applyS(i);
        result.applyS(i);
        break;
      case 'y': // stab by Y = |0> + i|1>
        result.applyH(i);
        result.applyS(i);
        break;
      case 'Y': // stab by -Y = |0> - i|1>
        result.applyH(i);
        result.applyS(i);
        result.applyS(i);
        result.applyS(i);
        break;
      default:;
      }
    }
  }
  return result;
}
const Statistics& SatEncoder::getStats() const { return stats; }

void SatEncoder::QState::applyCNOT(unsigned long control,
                                   unsigned long target) {
  if (target >= n || control >= n) {
    return;
  }
  for (std::size_t i = 0U; i < n; ++i) {
    r[i] ^= (x[i][control] * z[i][target]) * (x[i][target] ^ z[i][control] ^ 1);
    x[i][target]  = x[i][target] ^ x[i][control];
    z[i][control] = z[i][control] ^ z[i][target];
  }
}

void SatEncoder::QState::applyH(unsigned long target) {
  if (target >= n) {
    return;
  }
  for (std::size_t i = 0U; i < n; i++) {
    r[i] ^= x[i][target] * z[i][target];
    x[i][target] = x[i][target] ^ z[i][target];
    z[i][target] = x[i][target] ^ z[i][target];
    x[i][target] = x[i][target] ^ z[i][target];
  }
}

void SatEncoder::QState::applyS(unsigned long target) {
  if (target >= n) {
    return;
  }
  for (std::size_t i = 0U; i < n; ++i) {
    r[i] ^= x[i][target] * z[i][target];
    z[i][target] = z[i][target] ^ x[i][target];
  }
}
