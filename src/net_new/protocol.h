#include <vector>
#include <json.hpp>

using byte = unsigned char;
using json = nlohmann::json;

class NtProtocol
{
public:
    // Construct from raw packet bytes starting from `start`, write ending position into `end`
    NtProtocol(const byte *start, byte *&end);

    // Destructor should be virtual
    virtual ~NtProtocol();

    // Serialize current protocol layer and insert in front of `bytes`, which contains
    // raw packet bytes serialized from higher layer.
    virtual void to_bytes(std::vector<byte> &bytes) const;

    // Encode protocol detail as json
    virtual json to_json() const;
};
