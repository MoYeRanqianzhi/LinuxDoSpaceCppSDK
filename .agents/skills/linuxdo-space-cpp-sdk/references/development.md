# Development Guide

## Workdir

```bash
cd sdk/cpp
```

## Validate

Preferred direct validation:

```bash
g++ -std=c++17 -Iinclude src/LinuxDoSpace.cpp examples/basic.cpp -o linuxdospace-cpp-example
./linuxdospace-cpp-example
```

Alternative CMake validation:

```bash
cmake -S . -B build
cmake --build build
./build/linuxdospace_cpp_example
```

## Release model

- Workflow file: `../../../.github/workflows/release.yml`
- Trigger: push tag `v*`
- Current release output is a source archive uploaded to GitHub Release

## Keep aligned

- `../../../include/LinuxDoSpace.hpp`
- `../../../src/LinuxDoSpace.cpp`
- `../../../README.md`
- `../../../examples/basic.cpp`
- `../../../.github/workflows/ci.yml`
- `../../../.github/workflows/release.yml`

