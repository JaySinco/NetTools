#pragma once
#include "common.h"

class filter;
using p_filter = std::shared_ptr<filter>;

class filter
{
public:
    virtual bool test(const json &j) const = 0;

    virtual json to_json() const = 0;

    static p_filter from_str(const std::string &code);
};
