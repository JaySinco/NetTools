#pragma once
#include "transport.h"

class validator
{
public:
    virtual bool test(const json &j) const = 0;

    // virtual json to_json() const = 0;

    bool test(const packet &pac) const;

    static std::shared_ptr<validator> from_str(const std::string &code);
};
