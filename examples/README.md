# IStudio Example Corpus

This directory roots a growing corpus of IStudio example programs paired with their "golden" target-language emissions. Each subdirectory captures one scenario:

- `math_basics/` — arithmetic helpers lowering to scalar math in each backend.
- `generic_pair/` — generic struct usage with by-value helpers.
- `string_formatting/` — string building across runtime targets.

Every example includes:

- `program.ist`: the curated IStudio source.
- `generated/cpp/`: the current C++17/20 emitter output (header + source).
- `generated/java/`: the staging Java 21 translation.
- `generated/python/`: the Python 3.12+ baseline.

Together these samples lock in our cross-target mapping for functions, generics, structs, and strings. Future backend work should keep the files in sync, updating them whenever semantics change.
