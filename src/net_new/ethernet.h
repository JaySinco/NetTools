#pragma once
#include "protocol.h"
#include <map>

struct mac
{
    u_char b1, b2, b3, b4, b5, b6;

    bool operator==(const mac &rhs) const;
    bool operator!=(const mac &rhs) const;
    std::string to_str() const;
};

class ethernet : public protocol
{
public:
    ethernet() = default;
    ethernet(const u_char *start, const u_char *&end);
    ethernet(const mac &dest, const mac &source, const std::string &type);
    virtual ~ethernet();

    virtual void to_bytes(std::vector<u_char> &bytes) const override;
    virtual json to_json() const override;
    virtual std::string type() const override;
    virtual std::string succ_type() const override;

private:
    static std::map<u_short, std::string> type_dict;

    struct detail
    {
        mac dest;      // Destination address
        mac source;    // Source address
        u_short type;  // Ethernet type
    } d;
};
