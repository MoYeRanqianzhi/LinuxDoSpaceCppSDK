# Consumer Guide

Use this SDK when your C++ application already owns the `/v1/token/email/stream`
transport and needs local parsing plus mailbox routing.

## Build / integrate

Preferred local build:

```bash
cmake -S . -B build
cmake --build build
```

Minimal direct compile path:

```bash
g++ -std=c++17 -Iinclude src/LinuxDoSpace.cpp examples/basic.cpp -o linuxdospace-cpp-example
```

Current release assets are source archives, so prefer source integration or
tag-pinned vendoring instead of package-manager wording.

## Core usage

```cpp
LinuxDoSpace::Client client("lds_pat...");
auto catch_all = client.bindRegex(".*", LinuxDoSpace::Suffix::linuxdo_space, true);
auto alice = client.bindExact("alice", LinuxDoSpace::Suffix::linuxdo_space, false);

client.ingestNdjsonLine(line);

LinuxDoSpace::MailMessage msg;
if (alice->listenNext(msg)) {
  std::cout << msg.subject << std::endl;
}

alice->close();
catch_all->close();
client.close();
```

## Key semantics

- `Client::listenNext(...)` is the full-stream queue.
- `Mailbox::listenNext(...)` is the mailbox queue.
- Mailbox queues do not backfill before first mailbox listen.
- `Mailbox` does not auto-close on destruction.
- `Client::route(...)` is local matching only.

