# IStudio Compiler – Complete Work Plan, Architecture, and Master AI Prompt

> Goal: Build **IStudio** — a powerful, general‑purpose language with clean syntax, strong typing, compile‑time computation, JIT/AOT options, GC, concurrency, and **multi‑target code generation** (C++, Java, Python, MATLAB, Web/TS/JS, etc.). The compiler is **fully modular**: lexer → parser → semantic analysis → IR → optimizations → translation/backends → packaging. It accepts **language rules**, **key parts**, **standard library in IStudio**, a **project directory**, and **output language flags**.

---

## 1) Vision & Scope

**Vision.** IStudio makes high‑level, safe, and expressive programming productive across domains (web, scientific, embedded) while emitting production‑grade code in existing ecosystems.

**Primary outcomes.**

* Parse and type‑check IStudio sources.
* Support C++‑style **compile‑time computation** (constexpr/metaprogramming analogs).
* Generate optimized code for requested targets via **compiler flags**.
* Ship a minimal **std library** written in IStudio that cross‑targets backends.
* Provide ergonomic tooling: formatter, linter, LSP, package manager, test runner.

**Non‑goals (v1).** Native GC implementation (use Boehm/ARC options), whole‑program global optimizer (limit to modular opts), full reflection of foreign runtimes (stage later).

---

## 2) Deliverables & Milestones (12–16 weeks, aggressive)

**M0 – Project Skeleton (Week 1)**

* Monorepo with CMake + Conan presets; `src/`, `runtime/`, `std/`, `examples/`.
* CLI `istudio` scaffolding; logging, error diagnostics, config loader.

**Progress (2024-10-17).**

* ✅ Repository skeleton established with CMake-based build (`CMakeLists.txt`, `src/`, `tests/`, `cmake/`, runtime/std/examples placeholders).
* ✅ CLI entry point `istudio` implemented with `--version` handling and shared warning policy.
* ✅ Minimal lexer, token/trivia model, AST context scaffolding, semantic, IR, opt, backend interfaces stubbed.
* ✅ Self-contained lexer smoke tests wired via CTest; CI workflow configured (GitHub Actions) to build and run tests on Linux.
* ✅ Authored `docs/grammar.ebnf` capturing core expression/statement grammar for the forthcoming Pratt parser.
* ✅ Implemented initial Pratt-style expression parser with assignment/binary/unary/call support plus parser-focused tests.
* 🔜 Stand up IR printing & constant folding scaffolds; extend parser to cover statements.

**M1 – Language Spec & Grammar (Weeks 1–2)**

* Language reference (EBNF) + typing rules + attribute/annotation system.
* Design docs for compile‑time computation (CTC) & macros/metaclasses.

**M2 – Front‑End (Weeks 2–5)**

* **Lexer** with trivia, source maps.
* **Parser** (Pratt or GLR) generating **AST**.
* **Symbol table** & **Name resolution**.
* **Type checker** (generics, traits/interfaces, sum/product types, optional/null‑safety).

**M3 – Middle IR (Weeks 4–6)**

* SSA‑ish typed IR (IST‑IR) with effect annotations.
* Lowering passes from AST → IR.

**M4 – Optimizations (Weeks 6–8)**

* Constant folding/prop, dead code elim, inlining heuristic, escape analysis (basic), purity‑aware CSE, loop‑simple opts.

**M5 – Backends (Weeks 7–12)**

* **C++17/20** generator (primary reference backend, high fidelity, exception model pluggable).
* **Java 21** generator.
* **Python 3.12+** generator (typed hints optional).
* **MATLAB** generator (function files + packages).
* **Web** generator: **TypeScript/JS** + optional **WASM** (via C++/Emscripten or MLIR/LLVM stage – Phase 2).

**M6 – Runtime & Stdlib (Weeks 8–12)**

* Core collections, option/result, async primitives, channels, math, io.
* Memory policy adapters (ARC/GC/arena) abstracted at IR level.

**M7 – Tooling (Weeks 10–14)**

* Formatter, linter, **LSP** (hover, go‑to, diagnostics), test runner, package manager (`istudio pm`).

**M8 – Integration & Samples (Weeks 12–16)**

* Showcase apps: web API, numeric kernels, CLI, plotting to MATLAB/JS, mixed‑lang project.
* Benchmarks & CI gates; doc site with tutorials.

---

## 3) Repository Layout

```
/istudio
  /cmake/                 # toolchain & presets
  /external/              # third‑party (header‑only, vendored)
  /docs/                  # spec, ADRs, design notes
  /src/
    /cli/                 # istudio main
    /front/               # lexer, parser, ast
    /sem/                 # symbols, types, resolver, checker
    /ir/                  # IST‑IR, passes
    /opt/                 # optimization passes
    /backends/
      /cpp/
      /java/
      /python/
      /matlab/
      /web/
    /support/             # diagnostics, logging, utils
    /plugins/             # extension points (syntax/macro/backends)
  /runtime/               # minimal runtime shims per target
  /std/                   # stdlib in IStudio
  /examples/
  /tests/
  /tools/                 # formatter, linter, LSP, pm
```

---

## 4) Command‑Line, Configs & Flags

**Invocations**

```
istudio build <project_dir> -t <target(s)> [options]
istudio run   <project_dir> -t <target> [--jit]
istudio fmt   <files>
istudio lsp   --stdio
```

**Key flags**

* `-t, --target`: `cpp`, `java`, `python`, `matlab`, `web` (comma‑sep for multi‑emit)
* `--opt-level`: `0..3` (O0..O3), `s` (size)
* `--memory`: `gc|arc|arena` (informs lowering/runtime stubs)
* `--ctc`: enable/require compile‑time execution
* `--module-out`: output dir per target
* `--stdlib`: path or builtin
* `--emit-ir`, `--emit-ast`, `--emit-cfg`, `--emit-diags-json`
* `--foreign`: allow foreign calls (C ABI/FFI model)
* `--null-safety`: `strict|lenient|off`

**Project manifest** `istudio.toml`

```toml
[project]
name = "simlab"
version = "0.1.0"

[compiler]
std = "latest"
null_safety = "strict"
memory = "gc"

[target.cpp]
std = "c++20"

[target.java]
version = "21"

[target.python]
version = "3.12"

[target.web]
type = "ts"
```

---

## 5) Language Highlights (Spec Sketch)

**Types.** `bool, i8..i64, u8..u64, f16..f64, decimal, char, string, array[T], vector[N,T], tuple, option[T], result[T,E], func, trait, struct, enum, class`.

**Generics & Traits.** Parametric types, trait bounds, where‑clauses.

**Ownership & Safety.** Default immutable bindings; `mut` keyword for mutation; linear/affine hints optional; null‑safety via `option[T]` and `?` elvis/coalesce.

**Concurrency.** `spawn`, `async/await`, channels, actors; deterministic structured concurrency.

**CTC (Compile‑Time Computation).** `ct fn`, `ct if`, `ct foreach`, compile‑time evaluation over AST/Types; metaclasses for generating members; hygienic macros.

**Modules/Packages.** `module a.b.c`; visibility `pub`, `internal`.

**Attributes.** `@inline`, `@noescape`, `@ffi("C")`, `@target("python")`, `@parallel(n=8)`.

**Error Handling.** `result[T,E]` with `try`/`?` propagation; interop lowers to exceptions in Java, to Eithers in TS, to exceptions/Status in C++ per policy.

**Example (sketch)**

```istudio
module math.lin

pub fn dot(xs: array[f64], ys: array[f64]) -> f64 {
  assert(len(xs) == len(ys))
  var acc: f64 = 0
  for i in 0..len(xs) { acc += xs[i] * ys[i] }
  return acc
}

ct fn vec_len[N: usize]() -> usize { return N }
```

---

## 6) Front‑End Architecture

**Lexer.** Token kinds with channel separation (code/comments), UTF‑8, trivia preserving, token spans.

**Parser.** Pratt parser with precedence table; produces lossless AST; error recovery via panic modes & expected‑set heuristics.

**AST.** Immutable nodes with `NodeId`, `Span`, `children[]`, `kind` enum.

**Symbol Table.** Scopes (file, module, type, block), overload sets, generic params.

**Types.** Unification engine, constraint solver, trait bound checker.

**Diagnostics.** Rich spans, notes, fix‑its, code actions; JSON for tooling; colored CLI.

---

## 7) Middle IR (IST‑IR)

**Goals.** Target‑agnostic, typed, SSA‑inspired with effects and memory model tags.

**Core elements.**

* Blocks, Ops, Values; dominance tree; side‑effect flags (read/write/io/alloc).
* Types mapped from IStudio types with variance info.
* Attributes for hints: `@vectorize`, `@inline`, `@noalloc`.

**Lowering stages.** AST → HIR (structured) → MIR (control‑flow explicit) → IST‑IR SSA.

**Analyses.** Liveness, aliasing (shallow), purity, escape, loop info, range.

---

## 8) Optimization Passes

* Const fold/prop
* DCE & DSE
* Inlining (budgeted)
* Common subexpr elision with effect guards
* Loop‑invariant code motion (LICM)
* Strength reduction, simple vectorization hooks
* Interop lowering (FFI stubs) late

---

## 9) Backend Translation Contracts

**Backend interface (C++ pseudocode)**

```cpp
struct BackendCtx {
  const IRModule& mod;
  const TargetProfile& profile; // e.g., C++20, Java21, Py312
  FileSystem& out;
  Diag& diag;
};

class Backend {
public:
  virtual string name() const = 0;
  virtual void emit(const BackendCtx&) = 0;
};
```

**Mapping rules**

* Functions → methods/free functions per target idioms
* Generics → C++ templates / Java generics / Python type params (PEP‑695 style) / TS generics / MATLAB param docs
* Errors → exceptions or sum types per policy
* Async → C++ coroutines / Java CompletableFuture / Python async/await / JS Promise
* Collections → stdlib adapters per target

**Codegen strategy**

* Template‑driven emit with Mustache‑like templates + small writers
* Pretty‑printer with line‑wrapping & import consolidation

---

## 10) Runtime & Memory Policies

* **GC**: Boehm or target’s GC (JVM/.NET). For Python/JS, rely on host.
* **ARC**: reference‑counted stubs on C++/ObjC targets.
* **Arena**: region alloc with lifetime scopes, no cycles.

Configured via `--memory` and per‑module overrides.

---

## 11) Stdlib (IStudio)

* `core`: option/result, prelude, traits, math
* `collections`: vec, map, set, deque
* `concurrency`: channels, actor, async
* `io`: files, net, http client (feature‑gated)
* `numeric`: linalg (thin), stats, random

Each module annotated with `@target` shims when necessary.

---

## 12) Compile‑Time Computation & Metaprogramming

**CT execution engine.** Interprets a safe subset of IStudio at compile time.

**Metaclasses.** Type transformers that can add fields/methods/derive impls.

**Macros.** Hygienic, AST‑level; pattern → template with type awareness.

**Examples**

```istudio
ct fn derive_eq[T]() { /* generate eq impl using fields */ }

@metaclass(derive_eq)
struct Point { x: f64, y: f64 }
```

---

## 13) Error Handling & Diagnostics

* Primary span + related spans + notes + suggestions
* Error codes (IST‑E####) for triage
* `--emit-diags-json` for editors/CI

---

## 14) LSP, Formatter, Linter

* **LSP**: hover types, go‑to def, ref search, rename, code actions from diagnostics, semantic tokens.
* **Formatter**: stable style; operator spacing; import grouping.
* **Linter**: idioms, perf hints, unsafe API usage, target portability.

---

## 15) Testing & CI

* **Unit tests** for lexer/parser/typer/IR.
* **Golden tests** for diagnostics and codegen outputs.
* **Round‑trip**: IStudio → target → compile/run (where applicable) → assert.
* **Performance**: microbench (dot product, map/reduce), memory stress.
* **CI matrix**: Ubuntu/Windows/macOS; GCC/Clang/MSVC; Python/Node/Java.

---

## 16) Example Targets & Emission

**C++ (O2, ARC):**

```
istudio build examples/lin --target=cpp --opt-level=2 --memory=arc
```

**Java:**

```
istudio build examples/webapi --target=java
```

**Python + Web:**

```
istudio build examples/viz --target=python,web
```

**Artifacts**

```
/out/cpp/... .hpp/.cpp
/out/java/... .java + pom/gradle hints
/out/python/... .py + pyproject stub
/out/matlab/... .m
/out/web/... .ts/.js + package.json stub
```

---

## 17) Extension & Plugin System

**Interfaces**

* Syntax extensions (new literals/keywords guarded by feature flags)
* Macro packs
* New backends
* Custom passes (pre‑/post‑lowering)

**Plugin manifest** `istudio-plugin.toml`

```toml
[name]
id = "company.linalg"
version = "0.1.0"
[kind]
backend = true
```

---

## 18) Security, Sandboxing, and CTC Safety

* CTC runs in sandbox with time/memory limits
* Disallow IO/Net in CTC unless explicitly allowed via `@allow(ct_io)`
* Deterministic CTC mode for reproducible builds

---

## 19) Risk Register & Mitigations

* **Parser ambiguity** → adopt precedence tests + GLR fallback for rare cases
* **Backend drift** → spec’d mapping tables + golden tests per target
* **CTC complexity** → phase‑1 subset; stage features
* **GC portability** → pluggable memory policy and shims

---

## 20) Success Metrics

* Parse+type‑check 100k LOC within 5s (release build)
* Emit C++/Java/Python for stdlib & samples with passing tests
* LSP latency < 50ms for hover on medium files

---

# MASTER AI PROMPT (for Code‑Gen Agents)

> Copy, fill placeholders, and paste into your AI tool (ChatGPT/Copilot/etc.). Use it to generate code, docs, and tests for IStudio.

```
SYSTEM
You are a senior compiler engineer building the IStudio language and toolchain. Follow the repo architecture and contracts below. Prefer correctness, modularity, and testability. Output production‑grade C++ with CMake. When details are ambiguous, propose options and pick the simplest that keeps extensibility.

USER CONTEXT
Project: IStudio – a general‑purpose language with JIT/AOT, static typing, GC/ARC/arena, concurrency, and multi‑target codegen (C++, Java, Python, MATLAB, Web/TS).
My environment: Ubuntu 24.04+, GCC 13+/Clang 18+, CMake 3.28+, Python 3.12+, Node 20+, Java 21.

TOP‑LEVEL GOALS
1) Implement modular phases: lexer → parser → symbols/types → IR → opts → backends.
2) Implement compile‑time computation (CTC) with a safe interpreter.
3) Implement backends (start with C++), using template‑driven emitters.
4) Provide CLI, diagnostics, formatter, and LSP scaffold.

REPO LAYOUT (must generate)
- cmake/, external/, docs/
- src/cli, src/front, src/sem, src/ir, src/opt, src/backends/{cpp,java,python,matlab,web}, src/support, src/plugins
- runtime/, std/, examples/, tests/, tools/

CODING STANDARDS
- C++20 minimum; no exceptions across module boundaries (use expected<T,E>);
- RAII; span/string_view; fmtlib for printing; gsl-lite for contracts.
- Use pimpl where helpful.

PHASE CONTRACTS (must implement headers + stubs + tests)
- Lexer: Token, TokenKind, LexerConfig, lex(source) → TokenStream with trivia + spans.
- Parser: Pratt parser; parse(tokens) → AST with NodeId, Span; error‑recovery; AST dump.
- Symbols/Types: interning, scopes, generics, trait bounds; type‑inference via unification.
- IR: IST‑IR typed SSA; IRBuilder; passes API; verifier.
- Opt: PassManager; DCE, ConstFold, Inliner, LICM, CSE.
- Backends: Backend interface; C++ emitter v0 mapping table; pretty‑printer; emitted file layout.
- Diagnostics: DiagCode enum; Reporter (spans + notes + fix‑its); JSON emitter.
- CLI: `istudio build/run/fmt/lsp`; flag parsing; project manifest reader.

BACKEND MAPPING TABLE (initial)
- functions → free funcs (C++), classes → struct+methods; generics → templates; async → coroutines; result[T,E] → tl::expected.

TEST PLAN (must create)
- golden tests for tokens, AST, diags, IR prints, codegen; round‑trip compile for C++.

TASK NOW
Generate the initial skeleton: CMake project, module headers, stubs, minimal lexer, token tests, CLI that prints version, and CI config for Linux.

OUTPUT FORMAT
- Provide a tree of files with concise contents; no ellipses.
- Include exact CMakeLists.txt, headers, and .cpp with minimal compilable code.
- Include a README with build instructions and next tasks.
```

---

## 21) “Enhance‑the‑Prompt” Suggestions (What to add for better AI results)

1. **Pin Versions & Toolchain** – exact compiler, CMake, Conan profile, fmt version.
2. **Acceptance Criteria per Phase** – explicit done‑ness (APIs stable, tests passing, coverage %).
3. **Mapping Tables** – per type/feature → backend constructs (e.g., `option[T]` → `std::optional<T>` in C++, `Optional<T>` in Java, `typing.Optional[T]` in Python).
4. **Small End‑to‑End Sample** – a tiny IStudio program + expected C++/Java/Python outputs to anchor generation.
5. **Performance Budgets** – lexer throughput, parser allocations, LSP latency.
6. **Error Codes Catalog** – reserve ranges per subsystem (e.g., IST‑E1000 lexer, 2000 parser...).
7. **Security Constraints** – CTC sandbox limits, no host IO by default; reproducible builds.
8. **Style Guides** – formatter rules and lints (imports, naming, comment density).
9. **Doc Stubs** – ADR templates (Architecture Decision Records) to track choices.
10. **Release Checklist** – versioning, CHANGELOG, semver policy, artifact signing.

---

## 22) Next Concrete Steps (Actionable)

* ✅ Initialize repo with the skeleton from the **MASTER AI PROMPT** (completed 2024-10-17).
* ✅ Implement tokens and a minimal lexer; add golden tests (completed 2024-10-17).
* ✅ Write EBNF for core expressions/statements; parser for expressions first (completed 2024-10-17).
* Stand up IR structs and printer; implement const folding.
* Build the C++ backend emitter for functions + structs + generics (templates).
* Add 3 example programs and their emitted C++/Java/Python to lock mapping.
* Wire CI and LSP scaffold; publish the spec draft and ADRs.

---

**End of plan.**
