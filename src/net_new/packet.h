#pragma once
#include <memory>
#include "protocol.h"

class packet
{
public:
    // Construct protocol stack from raw packet bytes
    packet(const u_char *start, const u_char *end);

    // Push protocol to stack
    void push_back(std::shared_ptr<protocol> prot);

    // Serialize protocol stack into raw packet `bytes`
    void to_bytes(std::vector<u_char> &bytes) const;

    // Stringfy protocol stack with user-defined line `prefix` and `delimiter`, based on
    // results return from NtProtocol::to_json
    std::string to_str(const std::string &prefix, const std::string &delimiter) const;

private:
    std::vector<std::shared_ptr<protocol>> stack;
};
