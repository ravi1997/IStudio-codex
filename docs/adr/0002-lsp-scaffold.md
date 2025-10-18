# ADR 0002 – Language Server Transport Scaffold
Status: Accepted  
Date: 2025-10-18

## Context

Tooling is a core deliverable for IStudio. Editors require an LSP endpoint to surface diagnostics, hovers, and future refactorings. Early milestones only need a transport scaffold that can evolve alongside semantic analysis without committing to a heavy dependency stack.

## Decision

- Implement a stdio-based JSON-RPC 2.0 server housed in `src/lsp/` with distinct components:
  - `MessageReader`/`MessageWriter` handle LSP framing (`Content-Length` headers).
  - `Server` consumes raw JSON payloads, performs lightweight parsing, and routes requests/notifications.
- For the first iteration:
  - Support `initialize`, `shutdown`, and `exit` with minimal capability advertisement (text sync only).
  - Return structured errors for unsupported methods.
  - Wire the server into the CLI as `istudio lsp`, using the same build target as the compiler.
- Avoid external JSON dependencies to keep builds offline-capable; use a minimal parser tuned for the supported methods.
- Add regression coverage via `tests/lsp/test_lsp.cpp` exercising the reader and lifecycle (`initialize` → `shutdown` → `exit`).
- Extend CI to ensure the new code path builds/tests across Linux, macOS, and Windows.

## Consequences

- Editors can connect immediately (even with limited functionality) while semantic/type infrastructure ships in upcoming milestones.
- Manual JSON handling keeps the dependency footprint small but will need to be revisited once we add complex payloads (hover data, diagnostics, semantic tokens).
- Embedding the language server into the core binary ensures consistent versioning but constrains future IPC/transport experiments.
- Tests assert protocol framing without relying on external libraries, preventing regressions as parsing logic evolves.

## Notes

- Follow-up ADR will determine whether to adopt a full JSON library or custom AST-to-LSP translation once diagnostic streaming is implemented.
- Server capabilities described here align with the spec draft (§11 Tooling Contracts).
