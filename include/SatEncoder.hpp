#pragma once

#include "CircuitOptimizer.hpp"
#include "QuantumComputation.hpp"

#include <chrono>
#include <iostream>
#include <locale>
#include <nlohmann/json.hpp>
#include <z3++.h>

using json = nlohmann::json;

class SatEncoder {
public:
    /**
     * Takes two Clifford circuits, constructs SAT instance and checks if there is an assignment that leads to
     * outputs that differ.
     * @param circuit first circuit
     * @param circuitTwo second circuit
     * @param inputs input states to consider. In stabilizer representation, e.g. ZZ == |00>. If empty all-zero state is
     * assumed.
     */
    bool
    testEqual(qc::QuantumComputation &circuit, qc::QuantumComputation &circuitTwo, std::vector<std::string> &inputs);

    /**
     * Constructs SAT instance for input circuit and checks satisfiability for given inputs
     * @param circuitOne circuit to construct SAT instance for
     * @param inputs input states to consider. In stabilizer representation, e.g., ZZ == |00>. If empty all-zero state is
     * assumed.
     */
    void checkSatisfiability(qc::QuantumComputation &circuitOne, std::vector<std::string> &inputs);

    [[nodiscard]] json to_json() const { return stats.to_json(); }

private:
    struct Statistics {
        std::size_t nrOfGates = 0U;
        std::size_t nrOfQubits = 0U;
        std::size_t nrOfSatVars = 0U;
        std::size_t nrOfGenerators = 0U;
        std::size_t nrOfFunctionalConstr = 0U;
        std::size_t circuitDepth = 0U;
        std::size_t nrOfDiffInputStates = 0U;
        std::map<std::string, double> z3StatsMap;
        bool equal = false;
        bool satisfiable = false;
        std::size_t preprocTime = 0U;
        std::size_t solvingTime = 0U;
        std::size_t satConstructionTime = 0U;

        [[nodiscard]] json to_json() const;

        void from_json(const json &j);
    };

    class QState {
        unsigned long n;
        std::vector<std::vector<bool>> x;
        std::vector<std::vector<bool>> z;
        std::vector<int> r;
        std::size_t prevGenId;

    public:
        void SetN(unsigned long n);

        [[nodiscard]] const std::vector<std::vector<bool>> &GetX() const;

        void SetX(const std::vector<std::vector<bool>> &x);

        void SetZ(const std::vector<std::vector<bool>> &z);

        void SetR(const std::vector<int> &r);

        [[nodiscard]] const std::size_t &GetPrevGenId() const;

        void SetPrevGenId(const std::size_t &prev_gen_id);

        [[nodiscard]] std::vector<std::vector<bool>> getLevelGenerator() const;

        void applyCNOT(unsigned long control, unsigned long target);

        void applyH(unsigned long target);

        void applyS(unsigned long target);

        void printStateTableau();
    };

    class CircuitRepresentation {
    public:
        std::vector<std::map<std::size_t, std::size_t>> generatorMappings; // list of generatorId <> generatorId maps. One map per level
        std::map<std::size_t, std::vector<std::vector<bool>>> idGeneratorMap;    // id <> generator map
    };

    std::map<std::vector<std::vector<bool>>, std::size_t> generators; // generator <> id map for reverse lookup

    static QState initializeState(unsigned long nrOfInputs, std::string input);

    static bool isClifford(qc::QuantumComputation &qc);

    SatEncoder::CircuitRepresentation preprocessCircuit(qc::DAG &dag, std::vector<std::string> &inputs);

    void constructSatInstance(SatEncoder::CircuitRepresentation &circuitRepresentation,
                              z3::solver &solver); // construct z3 instance. Assumes prepocessCircuit() has been run before.
    void constructMiterInstance(SatEncoder::CircuitRepresentation &circuitOneRepresentation,
                                SatEncoder::CircuitRepresentation &circuitTwoRepresentation,
                                z3::solver &solver); // assumes preprocess circuit has been run before

    bool isSatisfiable(z3::solver &solver);

    Statistics stats;
    std::size_t nrOfInputGenerators = 0U;
    std::size_t uniqueGenCnt = 0U;
};
