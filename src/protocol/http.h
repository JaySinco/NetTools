#pragma once
#include "protocol.h"

class http : public protocol
{
public:
    struct detail
    {
        std::string op;                             // Request or Response
        std::string ver;                            // Protocol version
        std::string method;                         // Request method
        std::string url;                            // Request url
        int status;                                 // Response status
        std::string msg;                            // Response message
        std::map<std::string, std::string> header;  // Key-value header
        std::string body;                           // Content body
    };

    http() = default;

    http(const u_char *const start, const u_char *&end, const protocol *prev = nullptr);

    virtual ~http() = default;

    virtual void to_bytes(std::vector<u_char> &bytes) const override;

    virtual json to_json() const override;

    virtual std::string type() const override;

    virtual std::string succ_type() const override;

    virtual bool link_to(const protocol &rhs) const override;

    const detail &get_detail() const;

private:
    detail d{};
};
