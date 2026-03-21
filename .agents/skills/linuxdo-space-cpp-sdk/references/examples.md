# Task Templates

## Consume one mailbox

```cpp
LinuxDoSpace::Client client("lds_pat...");
auto alice = client.bindExact("alice", LinuxDoSpace::Suffix::linuxdo_space, false);
client.ingestNdjsonLine(line);

LinuxDoSpace::MailMessage msg;
if (alice->listenNext(msg)) {
  std::cout << msg.address << std::endl;
}

alice->close();
client.close();
```

## Query local matches

```cpp
auto matches = client.route(msg.address);
```

## Important note

- Do not rely on `Mailbox` destruction for immediate unbind; call `close()`.

