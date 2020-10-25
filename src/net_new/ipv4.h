#include "protocol.h"

class NtP_IPv4 : public NtProtocol
{
public:
    NtP_IPv4(const byte *start, byte *&end);
    virtual ~NtP_IPv4();
    virtual void to_bytes(std::vector<byte> &bytes) const override;
    virtual json to_json() const override;
};
