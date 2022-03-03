#include "CircuitOptimizer.hpp"
#include "SatEncoder.hpp"
#include "algorithms/RandomCliffordCircuit.hpp"

#include <filesystem>
#include <gtest/gtest.h>
#include <locale>

class SatEncoderTest: public testing::TestWithParam<std::string> {
protected:
};

TEST_F(SatEncoderTest, CheckEqualWhenEqualRandomCircuits) {
    std::random_device        rd;
    std::mt19937              gen(rd());
    qc::RandomCliffordCircuit circOne(2, 1, gen());
    qc::CircuitOptimizer::flattenOperations(circOne);
    auto circTwo = circOne.clone();

    SatEncoder sat_encoder;
    bool       result = sat_encoder.testEqual(circOne, circTwo);
    EXPECT_EQ(result, true);
}

TEST_F(SatEncoderTest, CheckEqualWhenNotEqualRandomCircuits) {
    std::random_device        rd;
    std::mt19937              gen(rd());
    qc::RandomCliffordCircuit circOne(2, 1, gen());
    qc::CircuitOptimizer::flattenOperations(circOne);
    auto circTwo = circOne.clone();

    circTwo.erase(circTwo.begin());

    SatEncoder sat_encoder;
    bool       result = sat_encoder.testEqual(circOne, circTwo);
    EXPECT_EQ(result, false);
}

TEST_F(SatEncoderTest, CheckEqualWhenEqualRandomCircuitsWithInputs) {
    std::random_device        rd;
    std::mt19937              gen(rd());
    qc::RandomCliffordCircuit circOne(50, 10, gen());
    qc::CircuitOptimizer::flattenOperations(circOne);
    auto circTwo = circOne.clone();

    SatEncoder               sat_encoder;
    std::vector<std::string> inputs;
    inputs.emplace_back("ZX");
    inputs.emplace_back("ZZ");
    inputs.emplace_back("YZ");
    inputs.emplace_back("YY");
    inputs.emplace_back("XZ");
    std::string filename{};

    bool result = sat_encoder.testEqual(circOne, circTwo, inputs);
    EXPECT_EQ(result, true);
}

/* Benchmarking */
std::vector<std::string> getAllCompBasisStates(std::size_t nrQubits) {
    if (nrQubits == 1) {
        return {"I", "Z"};
    }
    std::vector<std::string> rest = getAllCompBasisStates(nrQubits - 1);
    std::vector<std::string> appended;
    for (const auto& s: rest) {
        appended.push_back(s + 'I');
        appended.push_back(s + 'Z');
    }
    return appended;
}

#pragma clang diagnostic pop

class SatEncoderBenchmarking: public testing::TestWithParam<std::string> {
public:
    const std::string benchmarkFilesPath;
};

TEST_F(SatEncoderBenchmarking, GrowingNrOfQubitsForFixedDepth) { // scaling wrt #qubits
    try {
        // Paper Evaluation:
        // std::vector<std::size_t> depths = {10, 50, 250, 1000};
        std::vector<std::size_t> depths = {10, 50};
        for (unsigned long depth: depths) {
            // 10, 1: 50, 2:  1000, 3: 250
            std::size_t       nrOfQubits = 1U;
            const std::size_t stepsize   = 1U;
            // Paper Evaluation:
            // const std::size_t  maxNrOfQubits = 128;
            const std::size_t  maxNrOfQubits = 16;
            std::random_device rd;
            std::ostringstream oss;
            auto               t  = std::time(nullptr);
            auto               tm = *std::localtime(&t);
            oss << std::put_time(&tm, "%d-%m-%Y");
            auto filename = oss.str();

            std::ofstream outfile(benchmarkFilesPath + "QB-" + std::to_string(depth) + "-" + filename + ".json");
            outfile << "{ \"benchmarks\" : [";

            for (; nrOfQubits < maxNrOfQubits; nrOfQubits += stepsize) {
                SatEncoder               sat_encoder;
                for (size_t j = 0; j < 10; j++) { // 10 runs with same params for representative sample
                    qc::RandomCliffordCircuit circOne(nrOfQubits, depth, rd());
                    qc::CircuitOptimizer::flattenOperations(circOne);
                    if (nrOfQubits != 1U || j != 0U) {
                        outfile << ", ";
                    }
                    sat_encoder.checkSatisfiability(circOne);
                    outfile << sat_encoder.to_json().dump(2U);
                }
            }
            outfile << "]}";
            outfile.close();
        }
    } catch (std::exception& e) {
        std::cerr << "EXCEPTION THROWN" << std::endl;
        std::cout << e.what() << std::endl;
    }
}

TEST_F(SatEncoderBenchmarking, GrowingCircuitSizeForFixedQubits) { // scaling wrt to circuit size
    try {
        // Paper Evaluation:
        // std::vector<std::size_t> qubits = {5, 20, 65, 127};
        std::vector<std::size_t> qubits = {5, 20};
        for (unsigned long nrOfQubits: qubits) {
            std::size_t depth = 1U;
            // Paper Evaluation:
            // std::size_t        maxDepth = 500;
            std::size_t        maxDepth = 50U;
            const std::size_t  stepsize = 5U;
            std::random_device rd;
            std::ostringstream oss;
            auto               t  = std::time(nullptr);
            auto               tm = *std::localtime(&t);
            oss << std::put_time(&tm, "%d-%m-%Y");
            auto filename = oss.str();

            std::ofstream outfile(benchmarkFilesPath + "CS-" + std::to_string(nrOfQubits) + "-" + filename + ".json");
            outfile << "{ \"benchmarks\" : [";
            for (; depth <= maxDepth; depth += stepsize) {
                SatEncoder sat_encoder;
                for (size_t j = 0; j < 10; j++) { // 10 runs with same params for representative sample
                    qc::RandomCliffordCircuit circOne(nrOfQubits, depth, rd());
                    qc::CircuitOptimizer::flattenOperations(circOne);
                    if (depth != 1U || j != 0U) {
                        outfile << ", ";
                    }
                    sat_encoder.checkSatisfiability(circOne);
                    outfile << sat_encoder.to_json().dump(2U);
                }
            }
            outfile << "]}";
            outfile.close();
        }
    } catch (std::exception& e) {
        std::cerr << "EXCEPTION THROWN" << std::endl;
        std::cout << e.what() << std::endl;
    }
}

TEST_F(SatEncoderBenchmarking, GrowingCircuitSizeForFixedQubitsGenerators) { // generators wrt circsize
    try {
        std::vector<std::size_t> qubits = {1, 2, 3};
        for (unsigned long nrOfQubits: qubits) {
            std::size_t depth = 1U;
            // Paper Evaluation:
            // std::size_t        maxDepth = 100;
            std::size_t        maxDepth = 10U;
            const std::size_t  stepsize = 1U;
            std::random_device rd;
            std::ostringstream oss;
            auto               t  = std::time(nullptr);
            auto               tm = *std::localtime(&t);
            oss << std::put_time(&tm, "%d-%m-%Y");
            auto filename = oss.str();

            std::ofstream outfile(benchmarkFilesPath + "G-" + std::to_string(nrOfQubits) + "-" + filename + ".json");
            outfile << "{ \"benchmarks\" : [";
            for (; depth <= maxDepth; depth += stepsize) {
                SatEncoder sat_encoder;
                for (size_t j = 0; j < 10; j++) { // 10 runs with same params for representative sample
                    qc::RandomCliffordCircuit circOne(nrOfQubits, depth, rd());
                    qc::CircuitOptimizer::flattenOperations(circOne);
                    if (depth != 1U || j != 0U) {
                        outfile << ", ";
                    }
                    sat_encoder.checkSatisfiability(circOne);
                    outfile << sat_encoder.to_json().dump(2U);
                }
            }
            outfile << "]}";
            outfile.close();
        }
    } catch (std::exception& e) {
        std::cerr << "EXCEPTION THROWN" << std::endl;
        std::cout << e.what() << std::endl;
    }
}

TEST_F(SatEncoderBenchmarking, EquivalenceCheckingGrowingNrOfQubits) { // Equivalence Checking
    try {
        // Paper Evaluation:
        // const std::size_t  depth         = 1000;
        const std::size_t depth    = 100;
        std::size_t       qubitCnt = 4;
        const std::size_t stepsize = 4;
        // Paper Evaluation:
        // const std::size_t  maxNrOfQubits = 128;
        const std::size_t  maxNrOfQubits = 16;
        std::random_device rd;
        std::random_device rd2;
        std::random_device rd3;
        std::random_device rd4;
        std::ostringstream oss;
        std::mt19937       gen(rd());
        std::mt19937       gen2(rd());
        auto               t  = std::time(nullptr);
        auto               tm = *std::localtime(&t);
        oss << std::put_time(&tm, "%d-%m-%Y");
        auto timestamp = oss.str();

        std::ofstream outfile(benchmarkFilesPath + "EC-" + timestamp + ".json");
        outfile << "{ \"benchmarks\" : [";

        auto                            ipts = getAllCompBasisStates(5);
        std::uniform_int_distribution<> distr(0, 31);

        for (; qubitCnt < maxNrOfQubits; qubitCnt += stepsize) {
            std::cout << "Nr Qubits: " << qubitCnt << std::endl;
            SatEncoder               sat_encoder;
            std::vector<std::string> inputs;
            for (size_t j = 0; j < 18; j++) {
                inputs.emplace_back(ipts.at(distr(gen2)));
            }

            qc::RandomCliffordCircuit circOne(qubitCnt, depth, gen());
            qc::CircuitOptimizer::flattenOperations(circOne);
            auto circTwo = circOne.clone();
            if (qubitCnt != 4) {
                outfile << ", ";
            }
            sat_encoder.testEqual(circOne, circTwo, inputs); // equivalent case
            outfile << sat_encoder.to_json().dump(2U);
        }

        qubitCnt = 4;
        for (; qubitCnt < maxNrOfQubits; qubitCnt += stepsize) {
            std::cout << "Nr Qubits: " << qubitCnt << std::endl;
            SatEncoder               sat_encoder;
            std::vector<std::string> inputs;
            for (size_t k = 0; k < 18; k++) {
                inputs.emplace_back(ipts.at(distr(gen2)));
            }

            bool result;
            do {
                qc::RandomCliffordCircuit circThree(qubitCnt, depth, gen());
                qc::CircuitOptimizer::flattenOperations(circThree);
                auto                            circFour = circThree.clone();
                std::uniform_int_distribution<> distr2(0, circFour.size()); // random error location in circuit
                circFour.erase(circFour.begin() + distr2(gen));
                outfile << ", ";
                result = sat_encoder.testEqual(circThree, circFour, inputs); // non-equivalent case
                outfile << sat_encoder.to_json().dump(2U);
            } while (result);
        }
        outfile << "]}";
        outfile.close();
    } catch (std::exception& e) {
        std::cerr << "EXCEPTION THROWN" << std::endl;
        std::cout << e.what() << std::endl;
    }
}
