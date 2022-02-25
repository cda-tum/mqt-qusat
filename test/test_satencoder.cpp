#include "CircuitOptimizer.hpp"
#include "algorithms/RandomCliffordCircuit.hpp"
#include "satencoder/SatEncoder.hpp"

#include "gtest/gtest.h"
#include <filesystem>
#include <locale>

class SatEncoderPTest: public testing::TestWithParam<std::string> {
protected:
    std::string            test_example_dir = "../../examples/";
    qc::QuantumComputation circuitOne{};
    qc::QuantumComputation circuitTwo{};

    void SetUp() override {
        circuitOne.import(test_example_dir + GetParam() + ".qasm");
        circuitTwo.import(test_example_dir + GetParam() + ".qasm");
    }
};
class SatEncoderTest: public testing::TestWithParam<std::string> {
protected:
    std::string            test_example_dir = "../../examples/";
    qc::QuantumComputation circuitOne{};
    qc::QuantumComputation circuitTwo{};
};
TEST_F(SatEncoderTest, CheckEqualWhenEqualRandomCircuits) {
    std::random_device        rd;
    std::mt19937              gen(rd());
    qc::RandomCliffordCircuit circOne(2, 1, gen());
    qc::CircuitOptimizer::flattenOperations(circOne);
    auto circTwo = circOne.clone();

    SatEncoder               sat_encoder;
    std::vector<std::string> inputs;
    std::string              filename{};
    bool                     result = sat_encoder.testEqual(circOne, circTwo, inputs, filename);
    EXPECT_EQ(result, true);
}

TEST_F(SatEncoderTest, CheckEqualWhenNotEqualRandomCircuits) {
    std::random_device        rd;
    std::mt19937              gen(rd());
    qc::RandomCliffordCircuit circOne(2, 1, gen());
    qc::CircuitOptimizer::flattenOperations(circOne);
    auto circTwo = circOne.clone();

    circTwo.erase(circTwo.begin());

    SatEncoder               sat_encoder;
    std::vector<std::string> inputs;
    std::string              filename{};
    bool                     result = sat_encoder.testEqual(circOne, circTwo, inputs, filename);
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

    bool result = sat_encoder.testEqual(circOne, circTwo, inputs, filename);
    EXPECT_EQ(result, true);
}

INSTANTIATE_TEST_SUITE_P(SatEncoder, SatEncoderPTest,
                         testing::Values(
                                 "bell" //,
                                 //"ghz",
                                 //"simons"
                                 ),
                         [](const testing::TestParamInfo<SatEncoderTest::ParamType>& info) {
                             std::string name = info.param;
                             std::replace(name.begin(), name.end(), '-', '_');
                             std::stringstream ss{};
                             ss << name;
                             return ss.str(); });
TEST_P(SatEncoderPTest, CheckEqualWhenEqualNoInputs) {
    SatEncoder               sat_encoder;
    std::vector<std::string> inputs;
    std::string              filename{};
    bool                     result = sat_encoder.testEqual(circuitOne, circuitTwo, inputs, filename);
    EXPECT_EQ(result, true);
}
TEST_P(SatEncoderPTest, CheckEqualWhenEqualTwoInputs) {
    SatEncoder               sat_encoder;
    std::vector<std::string> inputs;

    inputs.emplace_back("ZX");
    inputs.emplace_back("ZZ");
    std::string filename{};
    bool        result = sat_encoder.testEqual(circuitOne, circuitTwo, inputs, filename);
    EXPECT_EQ(result, true);
}

TEST_P(SatEncoderPTest, CheckEqualWhenNotEqualTwoInputs) {
    SatEncoder               sat_encoder;
    std::vector<std::string> inputs;

    inputs.emplace_back("ZX");
    inputs.emplace_back("ZZ");
    std::string filename{};
    circuitTwo.erase(circuitTwo.begin());

    bool result = sat_encoder.testEqual(circuitOne, circuitTwo, inputs, filename);
    EXPECT_EQ(result, false);
}

TEST_P(SatEncoderPTest, SatWithoutInputs) {
    SatEncoder               sat_encoder;
    std::vector<std::string> inputs;
    std::string              file;
    sat_encoder.checkSatisfiability(circuitOne, inputs, file);
}

TEST_P(SatEncoderPTest, SatWithInputs) {
    SatEncoder               sat_encoder;
    std::vector<std::string> inputs;
    inputs.emplace_back("XX");
    inputs.emplace_back("Zz");
    inputs.emplace_back("xZ");
    std::string file;
    sat_encoder.checkSatisfiability(circuitOne, inputs, file);
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
class SatEncoderBenchmarking: public testing::TestWithParam<std::string> {
public:
    const std::string benchmarkFilesPath = "";
};
TEST_F(SatEncoderBenchmarking, GrowingNrOfQubitsForFixedDepth) { // scaling wrt #qubits
    try {
        std::vector<std::size_t> depths = {10, 50, 250, 1000};
        for (size_t i = 0; i < depths.size(); i++) {
            std::size_t        depth         = depths.at(i); // 10, 1: 50, 2:  1000, 3: 250
            std::size_t        nrOfQubits    = 1;
            const std::size_t  stepsize      = 1;
            const std::size_t  maxNrOfQubits = 128;
            std::random_device rd;
            std::ostringstream oss;
            auto               t  = std::time(nullptr);
            auto               tm = *std::localtime(&t);
            oss << std::put_time(&tm, "%d-%m-%Y");
            auto        filename = oss.str();
            std::string filep    = benchmarkFilesPath + "QB-" + std::to_string(depth) + "-" + filename + ".json";

            std::ofstream outfile(filep, std::fstream::app);
            outfile << "{ \"benchmarks\" : [";
            outfile.close();

            for (; nrOfQubits < maxNrOfQubits; nrOfQubits += stepsize) {
                SatEncoder               sat_encoder;
                std::vector<std::string> inputs;
                for (size_t j = 0; j < 10; j++) { // 10 runs with same params for representative sample
                    qc::RandomCliffordCircuit circOne(nrOfQubits, depth, rd());
                    qc::CircuitOptimizer::flattenOperations(circOne);
                    sat_encoder.checkSatisfiability(circOne, inputs, filep);
                }
            }

            std::ofstream outfile2(filep, std::fstream::app);
            outfile2 << "]}";
            outfile2.close();
        }
    } catch (std::exception& e) {
        std::cerr << "EXCEPTION THROWN" << std::endl;
        std::cout << e.what() << std::endl;
    }
}

TEST_F(SatEncoderBenchmarking, GrowingCircuitSizeForFixedQubits) { // scaling wrt to circuit size
    try {
        std::vector<std::size_t> qubits = {5, 20, 65, 127};
        for (size_t i = 0; i < qubits.size(); i++) {
            std::size_t        depth      = 1;
            std::size_t        maxDepth   = 500;
            std::size_t        nrOfQubits = qubits.at(i);
            const std::size_t  stepsize   = 5;
            std::random_device rd;
            std::ostringstream oss;
            auto               t  = std::time(nullptr);
            auto               tm = *std::localtime(&t);
            oss << std::put_time(&tm, "%d-%m-%Y");
            auto        filename = oss.str();
            std::string filep    = benchmarkFilesPath+"CS-" + std::to_string(nrOfQubits) + "-" + filename + ".json"; // adapt name accordingly

            std::ofstream outfile(filep, std::fstream::app);
            outfile << "{ \"benchmarks\" : [";
            outfile.close();
            std::vector<std::string> inputs;
            for (; depth <= maxDepth; depth += stepsize) {
                SatEncoder sat_encoder;
                for (size_t j = 0; j < 10; j++) { // 10 runs with same params for representative sample
                    qc::RandomCliffordCircuit circOne(nrOfQubits, depth, rd());
                    qc::CircuitOptimizer::flattenOperations(circOne);
                    sat_encoder.checkSatisfiability(circOne, inputs, filep);
                }
            }

            std::ofstream outfile2(filep, std::fstream::app);
            outfile2 << "]}";
            outfile2.close();
        }
    } catch (std::exception& e) {
        std::cerr << "EXCEPTION THROWN" << std::endl;
        std::cout << e.what() << std::endl;
    }
}

TEST_F(SatEncoderBenchmarking, GrowingCircuitSizeForFixedQubitsGenerators) { // generators wrt circsize
    try {
        std::vector<std::size_t> qubits = {2};
        for (size_t i = 0; i < qubits.size(); i++) {
            std::size_t        depth      = 1;
            std::size_t        maxDepth   = 100;
            std::size_t        nrOfQubits = qubits.at(i);
            const std::size_t  stepsize   = 1;
            std::random_device rd;
            std::ostringstream oss;
            auto               t  = std::time(nullptr);
            auto               tm = *std::localtime(&t);
            oss << std::put_time(&tm, "%d-%m-%Y");
            auto        filename = oss.str();
            std::string filep    = benchmarkFilesPath+"G-" + std::to_string(nrOfQubits) + "-" + filename + ".json"; // adapt name accordingly

            std::ofstream outfile(filep, std::fstream::app);
            outfile << "{ \"benchmarks\" : [";
            outfile.close();
            std::vector<std::string> inputs;
            for (; depth <= maxDepth; depth += stepsize) {
                SatEncoder sat_encoder;
                for (size_t j = 0; j < 20; j++) { // 10 runs with same params for representative sample
                    qc::RandomCliffordCircuit circOne(nrOfQubits, depth, rd());
                    qc::CircuitOptimizer::flattenOperations(circOne);
                    sat_encoder.checkSatisfiability(circOne, inputs, filep);
                }
            }

            std::ofstream outfile2(filep, std::fstream::app);
            outfile2 << "]}";
            outfile2.close();
        }
    } catch (std::exception& e) {
        std::cerr << "EXCEPTION THROWN" << std::endl;
        std::cout << e.what() << std::endl;
    }
}

TEST_F(SatEncoderBenchmarking, EquivalenceCheckingGrowingNrOfQubits) { // Equivalence Checking
    try {
        const std::size_t  depth         = 1000;
        std::size_t        qubitCnt      = 4;
        const std::size_t  stepsize      = 4;
        const std::size_t  maxNrOfQubits = 128;
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
        auto        timestamp = oss.str();
        std::string filename  = benchmarkFilesPath+"EC-" + timestamp + ".json";

        std::ofstream outfile(filename, std::fstream::app);
        outfile << "{ \"benchmarks\" : [";
        outfile.close();

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
            sat_encoder.testEqual(circOne, circTwo, inputs, filename); // equivalent case
        }

        qubitCnt = 4;
        for (; qubitCnt < maxNrOfQubits; qubitCnt += stepsize) {
            std::cout << "Nr Qubits: " << qubitCnt << std::endl;
            SatEncoder               sat_encoder;
            std::vector<std::string> inputs;
            for (size_t k = 0; k < 18; k++) {
                inputs.emplace_back(ipts.at(distr(gen2)));
            }

            bool result = false;
            do {
                qc::RandomCliffordCircuit circThree(qubitCnt, depth, gen());
                qc::CircuitOptimizer::flattenOperations(circThree);
                auto                            circFour = circThree.clone();
                std::uniform_int_distribution<> distr2(0, circFour.size()); // random error location in circuit
                circFour.erase(circFour.begin() + distr2(gen));
                result = sat_encoder.testEqual(circThree, circFour, inputs, filename);
            } while (result == true);
        }

        std::ofstream outfile2(filename, std::fstream::app);
        outfile2 << "]}";
        outfile2.close();
    } catch (std::exception& e) {
        std::cerr << "EXCEPTION THROWN" << std::endl;
        std::cout << e.what() << std::endl;
    }
}
