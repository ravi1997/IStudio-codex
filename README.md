# IStudio

## Overview
- Modular compiler toolchain skeleton for the IStudio language
- Components in place for lexer, AST context, semantic scaffolding, IR, optimization passes, backend interface, diagnostics, and plugin registry
- Command-line entry point with `--version` support
- Pratt-style parser covering expressions plus let/return/block statements
- Initial IST-IR printer and constant folding pass with dedicated tests
- Self-contained lexer, parser, and IR smoke tests runnable via CTest

## Getting Started
- Configure: `cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug`
- Build: `cmake --build build --config Debug`
- Test: `ctest --test-dir build --output-on-failure`

## Repository Layout
- `src/`: core implementation (cli, front, sem, ir, opt, backends, support, plugins)
- `tests/`: unit and golden tests (initial lexer coverage)
- `cmake/`: dependency helpers
- `.github/workflows/`: CI pipelines
- `docs/`, `runtime/`, `std/`, `tools/`: placeholders for future work
- `examples/`: curated sample programs with synchronized backend outputs (C++/Java/Python)

## Next Steps
1. Expose AST dump tooling and drive semantic/type analysis scaffolding
2. Flesh out IST-IR lowering plus verifier ahead of richer optimisations
3. Expand diagnostics and type system groundwork for statements/blocks
4. Implement the first code generation backend (C++) and extend tests accordingly
