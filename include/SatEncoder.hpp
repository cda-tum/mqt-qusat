#pragma once

#include "CircuitOptimizer.hpp"
#include "QuantumComputation.hpp"

#include <boost/dynamic_bitset.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
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
     * @param inputs input states to consider. In stabilizer representation, e.g. ZZ == |00>
     */
    bool testEqual(qc::QuantumComputation& circuit, qc::QuantumComputation& circuitTwo, std::vector<std::string>& inputs);
    /**
     * Constructs SAT instance for input circuit and checks satisfiability for given inputs
     * @param circuitOne circuit to construct SAT instance for
     * @param inputs input states to consider. In stabilizer representation, e.g., ZZ == |00>
     */
    void checkSatisfiability(qc::QuantumComputation& circuitOne, std::vector<std::string>& inputs);

    [[nodiscard]] json to_json() const { return stats.to_json(); }

private:
    struct Statistics {
        std::size_t                   nrOfGates            = 0U; // # gates in ckt
        std::size_t                   nrOfQubits           = 0U; //
        std::size_t                   nrOfSatVars          = 0U; // # sat variables
        std::size_t                   nrOfGenerators       = 0U; // # unique generators appearing in ckt for given inputs
        std::size_t                   nrOfFunctionalConstr = 0U; // # functional z3 constraint
        std::size_t                   circuitDepth         = 0U;
        std::size_t                   nrOfDiffInputStates  = 0U;
        std::map<std::string, double> z3StatsMap;
        bool                          equal               = false;
        bool                          satisfiable         = false;
        std::size_t                   preprocTime         = 0U;
        std::size_t                   solvingTime         = 0U;
        std::size_t                   satConstructionTime = 0U;
        [[nodiscard]] json            to_json() const;
        void                          from_json(const json& j);
    };

    class QState {
        unsigned long                        n;
        std::vector<boost::dynamic_bitset<>> x;
        std::vector<boost::dynamic_bitset<>> z;
        std::vector<int>                     r;
        boost::uuids::uuid                   prevGenId;

    public:
        void                                                      SetN(unsigned long n);
        [[nodiscard]] const std::vector<boost::dynamic_bitset<>>& GetX() const;
        void                                                      SetX(const std::vector<boost::dynamic_bitset<>>& x);
        void                                                      SetZ(const std::vector<boost::dynamic_bitset<>>& z);
        void                                                      SetR(const std::vector<int>& r);
        [[nodiscard]] const boost::uuids::uuid&                   GetPrevGenId() const;
        void                                                      SetPrevGenId(const boost::uuids::uuid& prev_gen_id);
        [[nodiscard]] std::vector<boost::dynamic_bitset<>>        getLevelGenerator() const;
        void                                                      applyCNOT(unsigned long control, unsigned long target);
        void                                                      applyH(unsigned long target);
        void                                                      applyS(unsigned long target);
        void                                                      printStateTableau();
    };
    class CircuitRepresentation {
    public:
        std::vector<std::map<boost::uuids::uuid, boost::uuids::uuid>>      generatorMappings; // list of generatorId <> generatorId maps. One map per level
        std::map<boost::uuids::uuid, std::vector<boost::dynamic_bitset<>>> idGeneratorMap;    // id <> generator map
    };
    std::map<std::vector<boost::dynamic_bitset<>>, std::size_t>        generators;                                                                                         // global generator <> integer (== value of symbolic encoding) map
    std::map<std::vector<boost::dynamic_bitset<>>, boost::uuids::uuid> generatorIdMap;                                                                                     // generator <> id map
    static QState                                                      initializeState(unsigned long nrOfInputs, std::string input);                                       // initialize circuit tableaus for states corresponding to inputs
    static bool                                                        isClifford(qc::QuantumComputation& qc);                                                             // true if circuit uses only clifford gates
    SatEncoder::CircuitRepresentation                                  preprocessCircuit(qc::DAG& dag, std::vector<std::string>& inputs);                                  // construct data structures needed for SAT encoding. Input expected as string of stabilizers, e.g. XZZX = +00+, default = Z...Z = 0...0
    void                                                               constructSatInstance(SatEncoder::CircuitRepresentation& circuitRepresentation, z3::solver& solver); // construct z3 instance. Assumes prepocessCircuit() has been run before.
    void                                                               constructMiterInstance(SatEncoder::CircuitRepresentation& circuitOneRepresentation, SatEncoder::CircuitRepresentation& circuitTwoRepresentation, z3::solver& solver);
    bool                                                               isSatisfiable(z3::solver& solver);
    Statistics                                                         stats;
    std::size_t                                                        nrOfInputGenerators = 0U;
    std::size_t                                                        uniqueGenCnt        = 0U;
};
