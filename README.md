# IStudio

## Overview
- Modular compiler toolchain skeleton for the IStudio language
- Components in place for lexer, AST context, semantic scaffolding, IR, optimization passes, backend interface, diagnostics, and plugin registry
- Command-line entry point with `--version` support
- Pratt-style expression parser with assignment/binary/unary/call coverage
- Self-contained lexer and parser smoke tests runnable via CTest

## Getting Started
- Configure: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug`
- Build: `cmake --build build --config Debug`
- Test: `ctest --test-dir build --output-on-failure`

## Repository Layout
- `src/`: core implementation (cli, front, sem, ir, opt, backends, support, plugins)
- `tests/`: unit and golden tests (initial lexer coverage)
- `cmake/`: dependency helpers
- `.github/workflows/`: CI pipelines
- `docs/`, `runtime/`, `std/`, `examples/`, `tools/`: placeholders for future work

## Next Steps
1. Extend the parser to cover statements/declarations and expose AST dumps for tooling
2. Expand semantic analysis with type information and diagnostics
3. Define the IST-IR format and add serialization/printing with const-folding hooks
4. Implement the first code generation backend (C++) and extend tests accordingly
