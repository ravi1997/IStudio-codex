# IStudio Language Specification (Draft)

> Status: Draft – updated 2025-10-18  
> Scope: Core syntax, semantics, and runtime contracts for the IStudio compiler pipeline.

This document captures the first complete pass over the IStudio language rules. It complements `docs/grammar.ebnf` (token/grammar reference) and will evolve as semantic analysis and backends mature.

## 1. Design Goals

- Strongly-typed, expression-oriented language targeting multiple ecosystems (C++, Java, Python, MATLAB, Web).
- Predictable performance: deterministic destructors, explicit async, pluggable memory policies (ARC, tracing GC, arenas).
- Ergonomic metaprogramming via compile-time computation (CTC), traits, and macros while keeping runtime lightweight.
- Tooling-first: formatter, linter, LSP, package manager, and project manifests integrate into the compiler front-end.

## 2. Lexical Structure

- Unicode source files encoded as UTF-8. Identifiers limited to ASCII in v1 for portable backend emission.
- Significant tokens: keywords (`module`, `fn`, `struct`, `let`, `async`, `await`, `match`, `return`, `if`, `else`, `loop`, `break`, `continue`, `defer`), literals (integer, floating, string, boolean), punctuation/operators as defined in `docs/grammar.ebnf`.
- Comments: `// line`, `/* block */`. Doc comments (`///`, `/** */`) preserved in trivia for tooling.
- Whitespace is insignificant except inside string literals; indentation is advisory for formatter.

## 3. Modules & Imports

- Source unit declared with `module <path>`; modules map to directory hierarchy.
- Imports use `use module::symbol` with optional aliasing (`as`). Wildcard imports (`use module::*`) resolved via explicit export lists.
- Visibility rules: `pub` exposes items to dependents; private by default.
- Compilation unit root defined via project manifest (`istudio.toml`, section TBD).

## 4. Types

- **Primitives**: `bool`, `i8/i16/i32/i64/i128`, `u8/.../u128`, `f32`, `f64`, `char`, `string`.
- **Composite**: tuples `(T, U)`, structs, enums (sum types with payloads), arrays `[T; N]`, slices `[]T`, option/result sugar (`option[T]`, `result[T, E]`).
- **Generics**: param lists `<T, U: Trait>` on structs, enums, functions, and impl blocks. Defaults allowed (`<T = i32>`).
- **Traits**: interface-style constraints with associated types and default methods. Trait objects `dyn Trait` subject to backend capabilities.
- **Type inference**: Hindley-Milner style with extensions for traits and associated types; local inference only (no higher-ranked types in v1).

## 5. Expressions & Statements

- Expressions follow Pratt precedence (see grammar). Key constructs:
  - Function calls `fn(args)`.
  - Member access `value.field` and method calls `value.method(args)`.
  - Control flow: `if/else`, `match`, `loop`, `while`, `for (pat in expr)`, `break`, `continue`.
  - `defer` executed at scope exit, stacking LIFO.
  - `async` blocks/functions produce `Future[T]`; `await` allowed in async contexts.
  - `try` expression sugar for propagating `result` errors.
- Statements at module scope: `let`, `const`, `fn`, `struct`, `enum`, `impl`, `extern`, `use`, `pub`.

## 6. Compile-Time Computation (CTC)

- `ct fn` introduces compile-time functions executed during semantic analysis.
- `constexpr` expressions evaluated eagerly when all operands are compile-time known.
- Metaclasses/macros operate through the CTC environment with sandboxed side effects (no IO, deterministic).
- Compile-time context exposes:
  - Reflection stubs (struct fields, traits).
  - Emission hooks for generated AST fragments inserted before lowering.

## 7. Memory & Ownership

- Default ownership: move semantics with implicit copy for `Copy` trait types.
- Borrowing: `&T` (immutable), `&mut T` (mutable) with lifetime inference; currently limited to lexical scopes.
- Memory policies are pluggable per compilation target: ARC (default), tracing GC (via Boehm), arenas (module-level).
- `drop` trait provides deterministic finalization; `defer`/scope guard semantics align with C++ RAII.

## 8. Concurrency & Async

- Futures: `async fn foo() -> T` lowers to `Future[T]`.
- Executors abstracted in runtime; language provides `spawn`, `join`, `select`, and channel primitives (`chan<T>`).
- `async`/`await` semantics map to coroutine/generator constructs on each backend (C++20 coroutines, Java virtual threads, Python `asyncio`, JS promises/WASM).
- Shared mutable state requires synchronization primitives from the stdlib (mutex, rwlock, atomics).

## 9. Error Handling

- Primary mechanism: `result[T, E]` with `?` operator sugar (`try expr` in grammer).
- Panic/abort reserved for irrecoverable errors; optional per target (exceptions in C++, exceptions in Java, `panic!` raising in Python).
- Diagnostics produced with codes `IST-E####` using `support::DiagnosticReporter`.

## 10. Interop & Backends

- Primary IR: IST-IR (SSA, typed, effect annotations). Backends translate IR modules into target-specific templates.
- Mapping table (v1):
  - Structs → `struct`/`class` (C++/Java), dataclasses (Python), MATLAB `classdef`.
  - Generics → C++ templates, Java generics (erasure aware), Python type hints.
  - `option[T]` → `std::optional<T>`, `java.util.Optional<T>`, `typing.Optional[T]`.
  - `result[T, E]` → `tl::expected<T, E>` (C++), sealed classes (Java), tuples/exceptions (Python), MATLAB struct.
- Foreign imports use `extern "target"` declarations; binding generators live under `src/plugins/`.

## 11. Tooling Contracts

- **Formatter**: deterministic whitespace/brace placement; configured via `tools/formatter`.
- **Linter**: rule engine over AST/semantic graph; warnings surfaced via diagnostics channel.
- **LSP**: stdio transport, JSON-RPC 2.0. Initial capabilities: text sync (open/close/change), hover stubs, diagnostics streaming.
- **Package manager**: resolves `istudio.toml` dependencies, integrates with build graph (future milestone).

## 12. Standard Library Outline

- Core modules: `istudio.core` (option/result, iterators), `collections`, `async`, `math`, `io`, `fmt`.
- Target-specific shims live in `runtime/` with bridging code for each backend.
- Testing harness built into stdlib (`std::test`), enabling golden tests per backend.

## 13. Compilation Pipeline Summary

1. **Lexing** – Token stream with trivia, preserved source spans.
2. **Parsing** – Pratt parser builds AST with node IDs/spans.
3. **Semantic Analysis** – symbol tables, type inference, trait resolution, CTC execution.
4. **IR Lowering** – AST → IST-IR, verifying types/effects.
5. **Optimization** – constant folding, DCE, inliner, LICM, CSE.
6. **Backends** – emit target code, package artifacts.

## 14. Open Questions

- Lifetime system extensions for higher-ranked traits and async lifetimes.
- Debug symbol emission per backend (DWARF/PDB/source maps).
- Deterministic metaprogramming sandbox API surface.
- Versioning and module stability guarantees for package manager.

---

Feedback on this draft lives under `docs/spec/` via ADRs and design notes. Future revisions will incorporate formal typing rules, detailed concurrency semantics, and full backend mapping tables.
