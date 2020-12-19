#pragma once
#include "packet.h"

class filter;
using p_filter = std::shared_ptr<filter>;

class filter
{
public:
    virtual bool test(const json &j) const = 0;

    bool test(const packet &pac) const;

    static p_filter from_str(const std::string &code);
};
