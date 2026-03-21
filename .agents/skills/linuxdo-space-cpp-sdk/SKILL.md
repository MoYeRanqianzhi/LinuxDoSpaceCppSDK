---
name: linuxdo-space-cpp-sdk
description: Use when writing or fixing C++ code that consumes or maintains the LinuxDoSpace C++ SDK under sdk/cpp. Use for source integration, ndjson ingestion, full-stream consumption, mailbox bindings, ordered matching, explicit close semantics, release artifact guidance, and local validation.
---

# LinuxDoSpace C++ SDK

Read [references/consumer.md](references/consumer.md) first for normal SDK usage.
Read [references/api.md](references/api.md) for exact public C++ API names.
Read [references/examples.md](references/examples.md) for task-shaped snippets.
Read [references/development.md](references/development.md) only when editing `sdk/cpp`.

## Workflow

1. Treat this SDK as a transport-agnostic core. The caller owns the HTTPS NDJSON transport.
2. The SDK root relative to this `SKILL.md` is `../../../`.
3. Preserve these invariants:
   - `Client::ingestNdjsonLine(...)` is the upstream ingestion entrypoint
   - `Client::listenNext(...)` is the full-stream consumer entrypoint
   - `Mailbox::listenNext(...)` is the mailbox consumer entrypoint
   - `Suffix::linuxdo_space` is semantic and resolves after `ready.owner_username`
   - exact and regex bindings share one ordered chain per suffix
   - `allowOverlap=false` stops at first match; `true` continues
   - mailbox queues activate on first `Mailbox::listenNext(...)`
   - `Mailbox` does not auto-close on destructor; callers must close explicitly or close the owning client
4. If behavior changes, keep headers, implementation, README, examples, and workflows aligned.
5. Validate with the commands in `references/development.md`.

## Do Not Regress

- Do not imply this SDK opens its own upstream network connection.
- Do not imply scope exit auto-unbinds mailbox bindings.
- Do not describe current release output as a package-manager install.
