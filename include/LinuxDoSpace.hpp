#ifndef LINUXDOSPACE_HPP
#define LINUXDOSPACE_HPP

#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

namespace LinuxDoSpace {

enum class Suffix {
  linuxdo_space
};

inline std::string toString(Suffix suffix) {
  if (suffix == Suffix::linuxdo_space) {
    return "linuxdo.space";
  }
  return "";
}

struct MailMessage {
  std::string address;
  std::string sender;
  std::vector<std::string> recipients;
  std::string receivedAt;
  std::string subject;
  std::string messageId;
  std::string date;
  std::string fromHeader;
  std::string toHeader;
  std::string ccHeader;
  std::string replyToHeader;
  std::vector<std::string> fromAddresses;
  std::vector<std::string> toAddresses;
  std::vector<std::string> ccAddresses;
  std::vector<std::string> replyToAddresses;
  std::string text;
  std::string html;
  std::string raw;
  std::vector<std::uint8_t> rawBytes;
};

class Error : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class AuthenticationError : public Error {
public:
  using Error::Error;
};

class StreamError : public Error {
public:
  using Error::Error;
};

class Client;

class Mailbox {
public:
  Mailbox() = default;
  ~Mailbox();
  Mailbox(const Mailbox &) = delete;
  Mailbox &operator=(const Mailbox &) = delete;
  Mailbox(Mailbox &&other) noexcept;
  Mailbox &operator=(Mailbox &&other) noexcept;

  bool listenNext(MailMessage &out);
  void close();
  bool closed() const;
  bool allowOverlap() const;
  bool isPattern() const;
  const std::string &suffix() const;
  const std::string &prefix() const;
  const std::string &pattern() const;

private:
  struct Impl;
  explicit Mailbox(std::shared_ptr<Impl> impl);
  std::shared_ptr<Impl> impl_;
  friend class Client;
};

class Client {
public:
  explicit Client(std::string token, std::string baseUrl = "https://api.linuxdo.space");
  ~Client();

  Client(const Client &) = delete;
  Client &operator=(const Client &) = delete;
  Client(Client &&) = delete;
  Client &operator=(Client &&) = delete;

  Mailbox bindExact(const std::string &prefix, const std::string &suffix, bool allowOverlap = false);
  Mailbox bindExact(const std::string &prefix, Suffix suffix, bool allowOverlap = false);
  Mailbox bindRegex(const std::string &pattern, const std::string &suffix, bool allowOverlap = false);
  Mailbox bindRegex(const std::string &pattern, Suffix suffix, bool allowOverlap = false);

  void ingestNdjsonLine(const std::string &line);
  bool listenNext(MailMessage &out);
  std::vector<Mailbox> route(const std::string &address) const;

  void close();
  bool closed() const;

private:
  struct Impl;
  std::shared_ptr<Impl> impl_;
};

} // namespace LinuxDoSpace

#endif
