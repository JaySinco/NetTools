#pragma once
#include "packet.h"

class validator;
using p_validator = std::shared_ptr<validator>;

class validator
{
public:
    virtual bool test(const json &j) const = 0;

    bool test(const packet &pac) const;

    static p_validator from_str(const std::string &code);
};
