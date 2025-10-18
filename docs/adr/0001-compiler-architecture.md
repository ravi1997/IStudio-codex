# ADR 0001 – Modular Compiler Architecture and Repository Layout
Status: Accepted  
Date: 2025-10-18

## Context

IStudio targets multiple backends (C++, Java, Python, MATLAB, Web) while offering advanced features—compile-time computation, traits, async, and pluggable memory policies. The implementation must stay maintainable across a growing team and support future tooling (formatter, LSP, package manager).

## Decision

- Adopt a phase-oriented architecture: `front` (lexer/parser/AST) → `sem` (symbols/types/CTC) → `ir` (IST-IR) → `opt` (passes) → `backends/{cpp,java,python,matlab,web}`.
- Package shared utilities under `support/` and extension points under `plugins/`.
- Use a monorepo with the following first-class directories:
  - `src/` for compiler sources grouped by subsystem.
  - `runtime/`, `std/`, `examples/` for generated artifacts and reference programs.
  - `docs/` for specs, ADRs, and design notes.
  - `tests/` for unit/golden tests orchestrated through CTest.
- Expose a single CLI entry point (`istudio`) with subcommands for build, run, fmt, lsp, etc.
- Track third-party dependencies via CMake `FetchContent` (header-only or vendored under `external/`) to keep builds hermetic.

## Consequences

- Phases have explicit contracts, enabling focused teams to iterate independently and making testing easier (each subdirectory owns its tests).
- Backends can be implemented incrementally without destabilising front-end work; IST-IR acts as the canonical interface.
- The repository layout matches the developer prompt and onboarding docs, keeping agents/tools aligned.
- Requires disciplined API design between phases (shared headers in `src/*`) and consistent CMake wiring to avoid cyclic dependencies.
- FetchContent relies on network access during configure; offline builds must vendor dependencies when necessary.

## Notes

- Supersedes any prior informal architecture sketches.
- Future ADRs will cover semantic analysis internals, IR evolution, and backend-specific strategies.
