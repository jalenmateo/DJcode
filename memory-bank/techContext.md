# Technical Context: DJcode Compiler

## Core Technology
-   **Language:** C (Standard C, likely C99 or C11 for compatibility).
-   **Compiler:** GCC or Clang (assuming a standard Linux/macOS/WSL environment, but should be portable).
-   **Build System:** GNU Make.

## Development Setup
-   A C compiler (GCC/Clang) and Make need to be installed.
-   Standard C libraries (`stdio.h`, `stdlib.h`, `string.h`, `stdint.h`, `math.h`) will be used.
-   No external libraries are planned initially, keeping dependencies minimal.

## Technical Constraints
-   Must run on standard systems with a C compiler and Make.
-   Memory usage should be reasonable, especially concerning the audio buffer size. The buffer size will depend on the length of the generated audio.
-   Performance is not a primary concern for V1, but the generation process should be reasonably fast for short sequences.

## Dependencies
-   Standard C Library
-   Math Library (`-lm` linker flag likely required for `math.h` functions like `cos`).
