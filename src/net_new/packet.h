#include <memory>
#include "protocol.h"

class NtPacket
{
public:
    // Construct protocol stack from raw packet bytes
    NtPacket(const byte *start, const byte *end);

    // Push protocol to stack
    void push_back(std::shared_ptr<NtProtocol> protocol);

    // Serialize protocol stack into raw packet `bytes`
    void to_bytes(std::vector<byte> &bytes) const;

    // Stringfy protocol stack with user-defined line `prefix` and `delimiter`, based on
    // results return from NtProtocol::to_json
    std::string to_str(const std::string &prefix, const std::string &delimiter) const;

private:
    std::vector<std::shared_ptr<NtProtocol>> stack;
};
