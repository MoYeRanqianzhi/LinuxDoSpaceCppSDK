# LinuxDoSpace C++ SDK

`sdk/cpp` provides a C++17 SDK for LinuxDoSpace mail-stream local routing.

## Scope

This SDK follows:

- `https://github.com/MoYeRanqianzhi/LinuxDoSpace/blob/main/sdk/spec/MAIL_STREAM_PROTOCOL.md`

Current core coverage:

- `Client` lifecycle (`close`, destruction-safe cleanup)
- `MailMessage` typed structure
- full stream intake queue (`listenNext`)
- local mailbox bindings:
  - exact prefix + suffix
  - regex + suffix
- one ordered match chain per suffix
- `allowOverlap` behavior

Important:

- `Suffix::linuxdo_space` is semantic, not literal
- the canonical default binding resolves to
  `<owner_username>-mail.linuxdo.space`
- `semanticSuffix(Suffix::linuxdo_space).withSuffix("foo")` resolves to
  `<owner_username>-mailfoo.linuxdo.space`
- the legacy default alias `<owner_username>.linuxdo.space` still matches the
  default semantic binding automatically
- local route inspection (`route`)
- mailbox close and queue activation semantics

## Important behavior

- Binding registers immediately.
- Mailbox queue starts after first `Mailbox::listenNext`.
- No pre-listen backlog is backfilled.
- Exact and regex are matched in one creation-order chain.

## Build

Requirements:

- C++17 compiler
- CMake 3.16+

Build steps:

```bash
cmake -S . -B build
cmake --build build
```

## Example

Example source:

- `examples/basic.cpp`

Run after build:

```bash
./build/linuxdospace_cpp_example
```

## Dependencies

- Standard library only for current core functionality.
- Transport is currently ingestion-oriented (`ingestNdjsonLine`) so caller can plug HTTPS streaming backend without changing local dispatch logic.

## Verification status (this environment)

- Will be compiled locally with `g++` in this task.
- Runtime HTTPS stream transport is intentionally separated from local routing core in this iteration.
