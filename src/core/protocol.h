#pragma once
#include "type.h"
#include <vector>

class protocol
{
public:
    // Destructor should be virtual
    virtual ~protocol(){};

    // Serialize current protocol layer and insert in front of `bytes`, which contains
    // raw packet bytes serialized from higher layer.
    virtual void to_bytes(std::vector<u_char> &bytes) const = 0;

    // Encode protocol detail as json
    virtual json to_json() const = 0;

    // Self protocol type
    virtual std::string type() const = 0;

    // Successor protocol type that follows
    virtual std::string succ_type() const = 0;
};
