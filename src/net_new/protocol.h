#include <vector>

typedef unsigned char byte;

class NtProtocol
{
public:
    // Construct from raw packet bytes starting from `data`, write ending position into `after`
    NtProtocol(const byte *data, byte *&after);

    // Destructor should be virtual
    virtual ~NtProtocol();

    // Serialize current protocol layer and insert bytes in front of `pkt_data`, which contains
    // raw packet bytes serialized from higher layer.
    virtual void to_bytes(std::vector<byte> &pkt_data) const;

    // Type of protocol
    virtual std::string type() const;

    // Description of protocol
    virtual std::string str() const;
};
