#pragma once
#include "type.h"
#include <vector>
#include <json.hpp>

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

    // Self protocol type
    virtual std::string type() const;

    // Successor protocol type that follows
    virtual std::string succ_type() const;
};
