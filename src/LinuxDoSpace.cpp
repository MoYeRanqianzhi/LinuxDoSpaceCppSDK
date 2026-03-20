#include "LinuxDoSpace.hpp"

#include <algorithm>
#include <cctype>
#include <functional>
#include <sstream>
#include <unordered_set>

namespace LinuxDoSpace {

namespace {

std::string trimCopy(std::string value) {
  while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
    value.erase(value.begin());
  }
  while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
    value.pop_back();
  }
  return value;
}

std::string lowerCopy(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return value;
}

std::optional<std::string> extractJsonString(const std::string &json, const std::string &key) {
  const std::string needle = "\"" + key + "\"";
  std::size_t pos = json.find(needle);
  if (pos == std::string::npos) {
    return std::string();
  }
  pos = json.find(':', pos + needle.size());
  if (pos == std::string::npos) {
    return std::nullopt;
  }
  pos = json.find('"', pos);
  if (pos == std::string::npos) {
    return std::string();
  }
  std::size_t end = pos + 1;
  while (end < json.size()) {
    if (json[end] == '"' && json[end - 1] != '\\') {
      break;
    }
    end++;
  }
  if (end >= json.size()) {
    return std::nullopt;
  }
  std::string raw = json.substr(pos + 1, end - pos - 1);
  std::string out;
  out.reserve(raw.size());
  for (std::size_t i = 0; i < raw.size(); i++) {
    if (raw[i] == '\\' && i + 1 < raw.size() && (raw[i + 1] == '\\' || raw[i + 1] == '"')) {
      i++;
    }
    out.push_back(raw[i]);
  }
  return out;
}

std::optional<std::string> extractJsonFirstArrayString(const std::string &json, const std::string &key) {
  const std::string needle = "\"" + key + "\"";
  std::size_t pos = json.find(needle);
  if (pos == std::string::npos) {
    return std::string();
  }
  pos = json.find('[', pos + needle.size());
  if (pos == std::string::npos) {
    return std::nullopt;
  }
  std::size_t quote = json.find('"', pos + 1);
  if (quote == std::string::npos) {
    return std::string();
  }
  std::size_t end = quote + 1;
  while (end < json.size()) {
    if (json[end] == '"' && json[end - 1] != '\\') {
      break;
    }
    end++;
  }
  if (end >= json.size()) {
    return std::nullopt;
  }
  return json.substr(quote + 1, end - quote - 1);
}

std::vector<std::string> extractJsonArrayStrings(const std::string &json, const std::string &key) {
  std::vector<std::string> out;
  const std::string needle = "\"" + key + "\"";
  std::size_t pos = json.find(needle);
  if (pos == std::string::npos) {
    return out;
  }
  pos = json.find('[', pos + needle.size());
  if (pos == std::string::npos) {
    return out;
  }
  pos++;
  while (pos < json.size()) {
    while (pos < json.size() && json[pos] != '"' && json[pos] != ']') {
      pos++;
    }
    if (pos >= json.size() || json[pos] == ']') {
      break;
    }
    std::size_t end = pos + 1;
    while (end < json.size()) {
      if (json[end] == '"' && json[end - 1] != '\\') {
        break;
      }
      end++;
    }
    if (end >= json.size()) {
      break;
    }
    out.push_back(lowerCopy(trimCopy(json.substr(pos + 1, end - pos - 1))));
    pos = end + 1;
  }
  return out;
}

int b64Val(char c) {
  if (c >= 'A' && c <= 'Z') return c - 'A';
  if (c >= 'a' && c <= 'z') return c - 'a' + 26;
  if (c >= '0' && c <= '9') return c - '0' + 52;
  if (c == '+') return 62;
  if (c == '/') return 63;
  if (c == '=') return -2;
  return -1;
}

std::optional<std::vector<std::uint8_t>> base64Decode(const std::string &input) {
  std::vector<std::uint8_t> out;
  out.reserve((input.size() / 4 + 1) * 3);
  std::size_t i = 0;
  while (i < input.size()) {
    while (i < input.size() && std::isspace(static_cast<unsigned char>(input[i]))) {
      i++;
    }
    if (i >= input.size()) {
      break;
    }
    if (i + 3 >= input.size()) {
      return std::nullopt;
    }
    int a = b64Val(input[i++]);
    int b = b64Val(input[i++]);
    int c = b64Val(input[i++]);
    int d = b64Val(input[i++]);
    if (a < 0 || b < 0 || c == -1 || d == -1) {
      return std::nullopt;
    }
    out.push_back(static_cast<std::uint8_t>((a << 2) | (b >> 4)));
    if (c != -2) {
      out.push_back(static_cast<std::uint8_t>(((b & 15) << 4) | (c >> 2)));
    }
    if (c != -2 && d != -2) {
      out.push_back(static_cast<std::uint8_t>(((c & 3) << 6) | d));
    }
  }
  return out;
}

std::string extractHeader(const std::string &raw, const std::string &key) {
  std::istringstream in(raw);
  std::string line;
  const std::string prefix = key + ":";
  while (std::getline(in, line)) {
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    if (line.empty()) {
      break;
    }
    if (line.rfind(prefix, 0) == 0) {
      return trimCopy(line.substr(prefix.size()));
    }
  }
  return "";
}

std::vector<std::string> parseAddresses(const std::string &header) {
  std::vector<std::string> out;
  std::stringstream ss(header);
  std::string part;
  while (std::getline(ss, part, ',')) {
    std::string token = trimCopy(part);
    auto lt = token.find('<');
    auto gt = token.find('>');
    if (lt != std::string::npos && gt != std::string::npos && gt > lt + 1) {
      token = token.substr(lt + 1, gt - lt - 1);
    }
    token = lowerCopy(trimCopy(token));
    if (!token.empty()) {
      out.push_back(token);
    }
  }
  return out;
}

std::string extractBody(const std::string &raw) {
  auto pos = raw.find("\r\n\r\n");
  if (pos != std::string::npos) {
    return raw.substr(pos + 4);
  }
  pos = raw.find("\n\n");
  if (pos != std::string::npos) {
    return raw.substr(pos + 2);
  }
  return "";
}

std::pair<std::string, std::string> splitAddress(const std::string &address) {
  auto at = address.find('@');
  if (at == std::string::npos || at == 0 || at + 1 >= address.size()) {
    return {"", ""};
  }
  return {lowerCopy(address.substr(0, at)), lowerCopy(address.substr(at + 1))};
}

} // namespace

struct Mailbox::Impl {
  bool closed{false};
  bool queueActive{false};
  bool allowOverlap{false};
  bool isPattern{false};
  std::function<void()> unregister;
  std::string suffix;
  std::string prefix;
  std::string pattern;
  std::regex regex;
  std::deque<MailMessage> queue;
};

struct Client::Impl : public std::enable_shared_from_this<Client::Impl> {
  std::string token;
  std::string baseUrl;
  bool closed{false};
  std::deque<MailMessage> allQueue;
  std::vector<std::shared_ptr<Mailbox::Impl>> bindings;

  bool mailboxMatches(const Mailbox::Impl &box, const std::string &address) const {
    if (box.closed) {
      return false;
    }
    auto parts = splitAddress(address);
    if (parts.first.empty() || parts.second.empty()) {
      return false;
    }
    if (parts.second != box.suffix) {
      return false;
    }
    if (!box.isPattern) {
      return parts.first == box.prefix;
    }
    return std::regex_match(parts.first, box.regex);
  }

  std::vector<std::shared_ptr<Mailbox::Impl>> resolveMatches(const std::string &address) const {
    std::vector<std::shared_ptr<Mailbox::Impl>> out;
    for (const auto &binding : bindings) {
      if (!mailboxMatches(*binding, address)) {
        continue;
      }
      out.push_back(binding);
      if (!binding->allowOverlap) {
        break;
      }
    }
    return out;
  }
};

Mailbox::Mailbox(std::shared_ptr<Mailbox::Impl> impl) : impl_(std::move(impl)) {}

Mailbox::~Mailbox() = default;
Mailbox::Mailbox(Mailbox &&other) noexcept : impl_(std::move(other.impl_)) {}
Mailbox &Mailbox::operator=(Mailbox &&other) noexcept {
  if (this != &other) {
    impl_ = std::move(other.impl_);
  }
  return *this;
}

bool Mailbox::listenNext(MailMessage &out) {
  if (!impl_ || impl_->closed) {
    return false;
  }
  impl_->queueActive = true;
  if (impl_->queue.empty()) {
    return false;
  }
  out = std::move(impl_->queue.front());
  impl_->queue.pop_front();
  return true;
}

void Mailbox::close() {
  if (!impl_) {
    return;
  }
  if (impl_->closed) {
    return;
  }
  impl_->closed = true;
  impl_->queue.clear();
  if (impl_->unregister) {
    impl_->unregister();
    impl_->unregister = {};
  }
}

bool Mailbox::closed() const { return !impl_ || impl_->closed; }
bool Mailbox::allowOverlap() const { return impl_ && impl_->allowOverlap; }
bool Mailbox::isPattern() const { return impl_ && impl_->isPattern; }
const std::string &Mailbox::suffix() const {
  static const std::string empty;
  return impl_ ? impl_->suffix : empty;
}
const std::string &Mailbox::prefix() const {
  static const std::string empty;
  return impl_ ? impl_->prefix : empty;
}
const std::string &Mailbox::pattern() const {
  static const std::string empty;
  return impl_ ? impl_->pattern : empty;
}

Client::Client(std::string token, std::string baseUrl) : impl_(std::make_shared<Impl>()) {
  token = trimCopy(token);
  if (token.empty()) {
    throw Error("token must not be empty");
  }
  impl_->token = std::move(token);
  impl_->baseUrl = trimCopy(std::move(baseUrl));
  if (impl_->baseUrl.empty()) {
    impl_->baseUrl = "https://api.linuxdo.space";
  }
}

Client::~Client() { close(); }

Mailbox Client::bindExact(const std::string &prefix, const std::string &suffix, bool allowOverlap) {
  if (impl_->closed) {
    throw Error("client is closed");
  }
  auto box = std::make_shared<Mailbox::Impl>();
  box->isPattern = false;
  box->allowOverlap = allowOverlap;
  box->suffix = lowerCopy(trimCopy(suffix));
  box->prefix = lowerCopy(trimCopy(prefix));
  std::weak_ptr<Impl> owner = impl_;
  box->unregister = [owner, box]() {
    auto sharedOwner = owner.lock();
    if (!sharedOwner) {
      return;
    }
    sharedOwner->bindings.erase(
        std::remove_if(
            sharedOwner->bindings.begin(),
            sharedOwner->bindings.end(),
            [box](const std::shared_ptr<Mailbox::Impl> &candidate) { return candidate == box; }),
        sharedOwner->bindings.end());
  };
  if (box->suffix.empty() || box->prefix.empty()) {
    throw Error("prefix and suffix must not be empty");
  }
  impl_->bindings.push_back(box);
  return Mailbox(box);
}

Mailbox Client::bindExact(const std::string &prefix, Suffix suffix, bool allowOverlap) {
  return bindExact(prefix, toString(suffix), allowOverlap);
}

Mailbox Client::bindRegex(const std::string &pattern, const std::string &suffix, bool allowOverlap) {
  if (impl_->closed) {
    throw Error("client is closed");
  }
  auto box = std::make_shared<Mailbox::Impl>();
  box->isPattern = true;
  box->allowOverlap = allowOverlap;
  box->suffix = lowerCopy(trimCopy(suffix));
  box->pattern = pattern;
  std::weak_ptr<Impl> owner = impl_;
  box->unregister = [owner, box]() {
    auto sharedOwner = owner.lock();
    if (!sharedOwner) {
      return;
    }
    sharedOwner->bindings.erase(
        std::remove_if(
            sharedOwner->bindings.begin(),
            sharedOwner->bindings.end(),
            [box](const std::shared_ptr<Mailbox::Impl> &candidate) { return candidate == box; }),
        sharedOwner->bindings.end());
  };
  if (box->suffix.empty() || box->pattern.empty()) {
    throw Error("pattern and suffix must not be empty");
  }
  try {
    box->regex = std::regex(box->pattern, std::regex::ECMAScript);
  } catch (const std::regex_error &e) {
    throw Error(std::string("invalid regex: ") + e.what());
  }
  impl_->bindings.push_back(box);
  return Mailbox(box);
}

Mailbox Client::bindRegex(const std::string &pattern, Suffix suffix, bool allowOverlap) {
  return bindRegex(pattern, toString(suffix), allowOverlap);
}

void Client::ingestNdjsonLine(const std::string &line) {
  if (impl_->closed) {
    throw Error("client is closed");
  }
  auto typeOpt = extractJsonString(line, "type");
  if (!typeOpt.has_value()) {
    throw StreamError("invalid JSON type field");
  }
  const std::string type = *typeOpt;
  if (type == "ready" || type == "heartbeat") {
    return;
  }
  if (type != "mail") {
    return;
  }

  auto senderOpt = extractJsonString(line, "original_envelope_from");
  auto recvOpt = extractJsonString(line, "received_at");
  auto recipients = extractJsonArrayStrings(line, "original_recipients");
  auto rawB64Opt = extractJsonString(line, "raw_message_base64");
  if (!senderOpt.has_value() || !recvOpt.has_value() || recipients.empty() || !rawB64Opt.has_value()) {
    throw StreamError("mail event missing required fields");
  }

  auto rawBytesOpt = base64Decode(*rawB64Opt);
  if (!rawBytesOpt.has_value()) {
    throw StreamError("invalid base64 payload");
  }
  std::string raw(rawBytesOpt->begin(), rawBytesOpt->end());
  std::string primaryRecipient = recipients.front();

  MailMessage msg;
  msg.address = primaryRecipient;
  msg.sender = *senderOpt;
  msg.recipients = recipients;
  msg.receivedAt = *recvOpt;
  msg.subject = extractHeader(raw, "Subject");
  msg.messageId = extractHeader(raw, "Message-ID");
  msg.date = extractHeader(raw, "Date");
  msg.fromHeader = extractHeader(raw, "From");
  msg.toHeader = extractHeader(raw, "To");
  msg.ccHeader = extractHeader(raw, "Cc");
  msg.replyToHeader = extractHeader(raw, "Reply-To");
  msg.fromAddresses = parseAddresses(msg.fromHeader);
  msg.toAddresses = parseAddresses(msg.toHeader);
  msg.ccAddresses = parseAddresses(msg.ccHeader);
  msg.replyToAddresses = parseAddresses(msg.replyToHeader);
  msg.text = extractBody(raw);
  msg.html = "";
  msg.raw = raw;
  msg.rawBytes = *rawBytesOpt;

  impl_->allQueue.push_back(msg);

  std::unordered_set<std::string> seenRecipients;
  for (const auto &recipient : recipients) {
    if (!seenRecipients.insert(recipient).second) {
      continue;
    }
    auto matches = impl_->resolveMatches(recipient);
    for (const auto &box : matches) {
      if (!box->closed && box->queueActive) {
        MailMessage perRecipient = msg;
        perRecipient.address = recipient;
        box->queue.push_back(std::move(perRecipient));
      }
    }
  }
}

bool Client::listenNext(MailMessage &out) {
  if (impl_->closed || impl_->allQueue.empty()) {
    return false;
  }
  out = std::move(impl_->allQueue.front());
  impl_->allQueue.pop_front();
  return true;
}

std::vector<Mailbox> Client::route(const std::string &address) const {
  if (impl_->closed) {
    throw Error("client is closed");
  }
  std::vector<Mailbox> out;
  auto matches = impl_->resolveMatches(address);
  out.reserve(matches.size());
  for (const auto &m : matches) {
    out.emplace_back(Mailbox(m));
  }
  return out;
}

void Client::close() {
  if (impl_->closed) {
    return;
  }
  impl_->closed = true;
  impl_->allQueue.clear();
  for (const auto &box : impl_->bindings) {
    box->closed = true;
    box->queue.clear();
    box->unregister = {};
  }
  impl_->bindings.clear();
}

bool Client::closed() const { return impl_->closed; }

} // namespace LinuxDoSpace
