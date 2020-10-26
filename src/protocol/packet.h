#pragma once
#include <memory>
#include "protocol.h"

class packet
{
public:
    // Construct protocol stack from raw packet bytes
    packet(const u_char *const start, const u_char *const end);

    // Push protocol in front of stack
    void push_front(std::shared_ptr<protocol> prot);

    // Serialize protocol stack into raw packet `bytes`
    void to_bytes(std::vector<u_char> &bytes) const;

private:
    // Low level -> High level
    std::vector<std::shared_ptr<protocol>> stack;
};
