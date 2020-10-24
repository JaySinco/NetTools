#include "protocol.h"

class NtP_IPv4 : public NtProtocol
{
public:
    NtP_IPv4(const byte *data, byte *&after);
    virtual ~NtP_IPv4();
    virtual void to_bytes(std::vector<byte> &pkt_data) const;
    virtual std::string type() const override;
    virtual std::string str() const override;
};
