# API Reference

## Paths

- SDK root: `../../../`
- Public header: `../../../include/LinuxDoSpace.hpp`
- Implementation: `../../../src/LinuxDoSpace.cpp`
- Consumer README: `../../../README.md`
- Build definition: `../../../CMakeLists.txt`

## Public surface

- Types: `Suffix`, `MailMessage`, `Error`, `AuthenticationError`, `StreamError`, `Mailbox`, `Client`
- Client:
  - `Client(...)`
  - `bindExact(...)`
  - `bindRegex(...)`
  - `ingestNdjsonLine(...)`
  - `listenNext(...)`
  - `route(...)`
  - `close()`
  - `closed()`
- Mailbox:
  - `listenNext(...)`
  - `close()`
  - `closed()`
  - `allowOverlap()`
  - `suffix()`
  - `prefix()`
  - `pattern()`

## Semantics

- This SDK is transport-agnostic; it does not own the HTTPS stream.
- Full-stream `MailMessage.address` is the first recipient projection.
- Mailbox delivery rewrites `address` to the matched recipient projection.
- Exact and regex bindings share one ordered chain per suffix.
- `Mailbox::~Mailbox()` does not imply immediate unbind.

