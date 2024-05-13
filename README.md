[![PyPI](https://img.shields.io/pypi/v/mqt.qusat?logo=pypi&style=flat-square)](https://pypi.org/project/mqt.qusat/)
![OS](https://img.shields.io/badge/os-linux%20%7C%20macos%20%7C%20windows-blue?style=flat-square)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg?style=flat-square)](https://opensource.org/licenses/MIT)
[![CI](https://img.shields.io/github/actions/workflow/status/cda-tum/mqt-qusat/ci.yml?branch=main&style=flat-square&logo=github&label=ci)](https://github.com/cda-tum/mqt-qusat/actions/workflows/ci.yml)
[![CD](https://img.shields.io/github/actions/workflow/status/cda-tum/mqt-qusat/cd.yml?style=flat-square&logo=github&label=cd)](https://github.com/cda-tum/mqt-qusat/actions/workflows/cd.yml)
[![codecov](https://img.shields.io/codecov/c/github/cda-tum/mqt-qusat?style=flat-square&logo=codecov)](https://codecov.io/gh/cda-tum/mqt-qusat)

> [!NOTE]
> This project is currently in low maintenance mode. We will still fix bugs and accept pull requests, but we will not
> actively develop new features.

<p align="center">
<a href="https://mqt.readthedocs.io">
    <picture>
      <source media="(prefers-color-scheme: dark)" srcset="https://raw.githubusercontent.com/cda-tum/mqt/main/docs/_static/mqt_light.png" width="60%">
      <img src="https://raw.githubusercontent.com/cda-tum/mqt/main/docs/_static/mqt_dark.png" width="60%">
    </picture>
  </a>
</p>

# MQT QuSAT - A Tool for Utilizing SAT in Quantum Computing

A tool for utilizing satisfiablity testing (SAT) techniques in quantum computing developed as part of the [_Munich Quantum Toolkit (MQT)_](https://mqt.readthedocs.io) by the [Chair for Design Automation](https://www.cda.cit.tum.de/) at the [Technical University of Munich](https://www.tum.de/) based on methods proposed in:

- [[1]](https://arxiv.org/abs/2203.00698) L. Berent, L. Burgholzer, and R. Wille. Towards a Satisfiability Encoding for Quantum Circuits. 2022.

QuSAT builds upon [MQT Core](https://github.com/cda-tum/mqt-core), which forms the backbone of the MQT.

The project can be used to

- Encode Clifford circuits in SAT
- Check the equivalence of Clifford circuits using SAT

If you have any questions, feel free to contact us via [quantum.cda@xcit.tum.de](mailto:quantum.cda@xcit.tum.de) or by creating an issue on [GitHub](https://github.com/cda-tum/mqt-qusat/issues).

## Towards a Satisfiability Encoding for Quantum Circuits

The results from the paper can be reproduced by first building the project as described below and then executing the resulting `qusat_test` executable in the build directory.
In order to replicate the full range of results, the `test/test_satencoder.cpp` needs to be modified before building the project.
The corresponding lines to be changed are marked with a `// Paper Evaluation:` comment.

Running the executable, produces several `.json` files containing the experimental data. The python script `/results/visualizer.py` can be used
to plot the respective data.

Note that, as we use a randomized procedure to generate input data, the exact experimental data will slightly vary every time the benchmarks are run.
The experimental data used in the paper is available in `/results` directory.

## System Requirements

Building (and running) is continuously tested under Linux, MacOS, and Windows using the [latest available system versions for GitHub Actions](https://github.com/actions/virtual-environments). However, the implementation should be compatible with any current C++ compiler supporting C++17 and a minimum CMake version of 3.19.

The SMT Solver [Z3 >= 4.8.3](https://github.com/Z3Prover/z3) has to be installed and the dynamic linker has to be able to find the library. This can be accomplished in a multitude of ways:

- Under Ubuntu 20.04 and newer: `sudo apt-get install libz3-dev`
- Under macOS: `brew install z3`
- Alternatively: `pip install z3-solver` and then append the corresponding path to the library path (`LD_LIBRARY_PATH` under Linux, `DYLD_LIBRARY_PATH` under macOS), e.g. via
  ```bash
  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(python -c "import z3; print(z3.__path__[0]+'/lib')")
  ```
- Download pre-built binaries from https://github.com/Z3Prover/z3/releases and copy the files to the respective system directories
- Build Z3 from source and install it to the system

## Configuration and Build

To start off, clone this repository using

```shell
git clone https://github.com/cda-tum/mqt-qusat --recursive
```

Note the `--recursive` flag. It is required to also clone all the required submodules.
If you happen to forget passing the flag on your initial clone, you can initialize all the submodules by executing `git submodule update --init --recursive` in the main project directory.

The project uses CMake as the main build configuration tool. Building a project using CMake is a two-stage process. First, CMake needs to be _configured_ by calling

```shell
cmake -S . -B build -DBUILD_MQT_QUSAT_TESTS=ON -DZ3_ROOT=/path/to/z3/
```

This tells CMake to search the current directory `.` (passed via `-S`) for a _CMakeLists.txt_ file and process it into a directory `build` (passed via `-B`). If your installation of Z3 is recent enough, the `Z3_ROOT` can typically be omitted.

After configuring with CMake, the library can be built by calling

```shell
cmake --build build
```

This tries to build the project in the `build` directory (passed via `--build`).
Some operating systems and developer environments explicitly require a configuration to be set, which is why the `--config` flag is also passed to the build command. The flag `--parallel <NUMBER_OF_THREADS>` may be added to trigger a parallel build.

# Reference

If you use our tool for your research, we would appreciate if you refer to it by citing the appropriate publication:

```
@inproceedings{berent2022sat,
      title={Towards a SAT Encoding for Quantum Circuits: A Journey From Classical Circuits to Clifford Circuits and Beyond},
      author={Lucas Berent and Lukas Burgholzer and Robert Wille},
      year={2022},
      booktitle={International Conference on Theory and Applications of Satisfiability Testing}
}
```

## Acknowledgements

The Munich Quantum Toolkit has been supported by the European
Research Council (ERC) under the European Union's Horizon 2020 research and innovation program (grant agreement
No. 101001318), the Bavarian State Ministry for Science and Arts through the Distinguished Professorship Program, as well as the
Munich Quantum Valley, which is supported by the Bavarian state government with funds from the Hightech Agenda Bayern Plus.

<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://raw.githubusercontent.com/cda-tum/mqt/main/docs/_static/tum_dark.svg" width="28%">
    <img src="https://raw.githubusercontent.com/cda-tum/mqt/main/docs/_static/tum_light.svg" width="28%" alt="TUM Logo">
  </picture>
  <picture>
    <img src="https://raw.githubusercontent.com/cda-tum/mqt/main/docs/_static/logo-bavaria.svg" width="16%" alt="Coat of Arms of Bavaria">
  </picture>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://raw.githubusercontent.com/cda-tum/mqt/main/docs/_static/erc_dark.svg" width="24%">
    <img src="https://raw.githubusercontent.com/cda-tum/mqt/main/docs/_static/erc_light.svg" width="24%" alt="ERC Logo">
    <img src="https://raw.githubusercontent.com/cda-tum/mqt/main/docs/_static/logo-mqv.svg" width="28%" alt="MQV Logo">
  </picture>
</p>
