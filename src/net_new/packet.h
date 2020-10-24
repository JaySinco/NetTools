#include <memory>
#include "protocol.h"

class NtPacket
{
public:
    NtPacket(const byte *);
    void push_back(std::shared_ptr<NtProtocol>);
    void to_bytes(std::vector<byte> &) const;
    std::string str() const;

private:
    std::vector<std::shared_ptr<NtProtocol>> stack;
};
