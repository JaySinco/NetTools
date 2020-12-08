#pragma once
#include "packet.h"

class validator
{
public:
    virtual bool operator()(const json &j) const = 0;

    bool test(const packet &pac) const;

    static std::shared_ptr<validator> from_str(const std::string &code);
};
