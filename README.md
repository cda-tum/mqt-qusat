# A Satisfiability Formulation Construction Algorithm for Clifford Quantum Circuits Written in C++
This project is a proof-of-principle implementation corresponding to the paper
[[1]]()
L. Berent, L. Burgholzer, and R. Wille. Towards a Satisfiability Encoding for Quantum Circuits. 2022.
## System Requirements

The implementation should be compatible with any current C++ compiler supporting C++17 and a minimum CMake version of 3.14.

`boost/program_options >= 1.50` is required for building the the commandline applications of the mapping tool.

The SMT Solver [Z3 >= 4.8.3](https://github.com/Z3Prover/z3) has to be installed and the dynamic linker has to be able to find the library. This can be
accomplished in a multitude of ways:

- Under Ubuntu 20.04 and newer: `sudo apt-get install libz3-dev`
- Under macOS: `brew install z3`
- Alternatively: `pip install z3-solver` and then append the corresponding path to the library path (`LD_LIBRARY_PATH` under Linux, `DYLD_LIBRARY_PATH` under macOS), e.g. via
    ```bash
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(python -c "import z3; print(z3.__path__[0]+'/lib')")
    ```
- Download pre-built binaries from https://github.com/Z3Prover/z3/releases and copy the files to the respective system directories
- Build Z3 from source and install it to the system


## Configure, Build, and Install

To start off, clone this repository using
```shell
git clone --recurse-submodules -j8 https://github.com/lucasberent/qsatencoder
```
Note the `--recurse-submodules` flag. It is required to also clone all the required submodules, several modules from the
QMAP toolkit https://github.com/iic-jku/qmap are needed.
If you happen to forget passing the flag on your initial clone, you can initialize all the submodules by executing `git submodule update --init --recursive` in the main project directory.

The project use CMake as the main build configuration tool. Building a project using CMake is a two-stage process. First, CMake needs to be *configured* by calling
```shell 
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```
This tells CMake to search the current directory `.` (passed via `-S`) for a *CMakeLists.txt* file and process it into a directory `build` (passed via `-B`).
The flag `-DCMAKE_BUILD_TYPE=Release` tells CMake to configure a *Release* build (as opposed to, e.g., a *Debug* build).

After configuring with CMake, the project can be built by calling
```shell
cmake --build build --config Release
```
This tries to build the project in the `build` directory (passed via `--build`).
Some operating systems and developer environments explicitly require a configuration to be set, which is why the `--config` flag is also passed to the build command. The flag `--parallel <NUMBER_OF_THREADS>` may be added to trigger a parallel build.

## Usage
After building the project, the benchmarks can be run by executing the corresponding test executable `qsatencoder_satencoder_test`.
```
qsatencoder_satencoder_test --gtest_filter=SatEncoderBenchmarking.*:SatEncoderBenchmarking/*.*:SatEncoderBenchmarking.*/*:*/SatEncoderBenchmarking.*/*:*/SatEncoderBenchmarking/*.*
```
This will produce several .json files containing the experimental data. The python script `/test/results/visualizer.py` can be used
to plot the respective data.

Note that as we use a randomized procedure to generate input data the exact experimental data will slightly vary everytime the benchmarks are run.
The experimental data used in the paper is available in the directory `/test/results`.
