#include "LinuxDoSpace.hpp"

#include <iostream>

int main() {
  using namespace LinuxDoSpace;

  Client client("lds_pat.demo");
  auto catchAll = client.bindRegex(".*", Suffix::linuxdo_space, true);
  auto alice = client.bindExact("alice", Suffix::linuxdo_space, false);

  MailMessage tmp;
  (void)catchAll.listenNext(tmp);
  (void)alice.listenNext(tmp);

  client.ingestNdjsonLine(
      "{\"type\":\"mail\",\"original_envelope_from\":\"bounce@example.com\","
      "\"original_recipients\":[\"alice@linuxdo.space\"],"
      "\"received_at\":\"2026-03-20T10:11:12Z\","
      "\"raw_message_base64\":\"RnJvbTogU2VuZGVyIDxzZW5kZXJAZXhhbXBsZS5jb20+DQpUbzogQWxpY2UgPGFsaWNlQGxpbnV4ZG8uc3BhY2U+DQpTdWJqZWN0OiBDUFAgVGVzdA0KDQpIZWxsbyBmcm9tIENQUD8=\"}");

  if (alice.listenNext(tmp)) {
    std::cout << "alice subject: " << tmp.subject << "\n";
  }
  if (catchAll.listenNext(tmp)) {
    std::cout << "catch-all address: " << tmp.address << "\n";
  }

  auto matches = client.route("alice@linuxdo.space");
  std::cout << "route matches: " << matches.size() << "\n";
  return 0;
}
