#pragma once
#include <vector>
#include <json.hpp>

#define Protocol_Type_Ethernet "Ethernet"
#define Protocol_Type_IPv4 "IPv4"

using u_char = unsigned char;
using u_short = unsigned short;
using u_int = unsigned int;
using u_long = unsigned long;
using json = nlohmann::json;

class protocol
{
public:
    // Destructor should be virtual
    virtual ~protocol();

    // Serialize current protocol layer and insert in front of `bytes`, which contains
    // raw packet bytes serialized from higher layer.
    virtual void to_bytes(std::vector<u_char> &bytes) const;

    // Encode protocol detail as json
    virtual json to_json() const;

    virtual std::string type() const;

    virtual std::string next_type() const;
};
