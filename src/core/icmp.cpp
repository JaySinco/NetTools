#include "icmp.h"

std::map<u_char, std::pair<std::string, std::map<u_char, std::string>>> icmp::type_dict = {
    {
        0,
        {"ping-reply", {}},
    },
    {
        3,
        {"error",
         {
             {0, "network unreachable"},
             {1, "host unreachable"},
             {2, "protocol unreachable"},
             {3, "port unreachable"},
             {4, "fragmentation needed but no frag. bit set"},
             {5, "source routing failed"},
             {6, "destination network unknown"},
             {7, "destination host unknown"},
             {8, "source host isolated (obsolete)"},
             {9, "destination network administratively prohibited"},
             {10, "destination host administratively prohibited"},
             {11, "network unreachable for TOS"},
             {12, "host unreachable for TOS"},
             {13, "communication administratively prohibited by filtering"},
             {14, "host precedence violation"},
             {15, "Pprecedence cutoff in effect"},
         }},
    },
    {
        4,
        {"error",
         {
             {0, "source quench"},
         }},
    },
    {
        5,
        {"error",
         {
             {0, "redirect for network"},
             {1, "redirect for host"},
             {2, "redirect for TOS and network"},
             {3, "redirect for TOS and host"},
         }},
    },
    {
        8,
        {"ping-ask", {}},
    },
    {
        9,
        {"router-notice", {}},
    },
    {
        10,
        {"router-request", {}},
    },
    {
        11,
        {"error",
         {
             {0, "ttl equals 0 during transit"},
             {1, "ttl equals 0 during reassembly"},
         }},
    },
    {
        12,
        {"error",
         {
             {0, "ip header bad (catch-all error)"},
             {1, "required options missing"},
         }},
    },
    {
        17,
        {"netmask-ask", {}},
    },
    {
        18,
        {"netmask-reply", {}},
    },
};

icmp::icmp(const u_char *const start, const u_char *&end)
{
    d = ntoh(*reinterpret_cast<const detail *>(start));
    extra.insert(extra.cend(), start + sizeof(detail), end);
}

icmp::icmp(const std::string &echo)
{
    d.type = 8;
    d.code = 0;
    d.u.s.id = rand_ushort();
    d.u.s.sn = rand_ushort();
    extra.insert(extra.cend(), echo.data(), echo.data() + echo.size());
}

void icmp::to_bytes(std::vector<u_char> &bytes) const
{
    auto dt = hton(d);
    size_t tlen = sizeof(detail) + extra.size();
    u_char *buf = new u_char[tlen];
    std::memcpy(buf, &dt, sizeof(detail));
    std::memcpy(buf + sizeof(detail), extra.data(), extra.size());
    dt.crc = calc_checksum(buf, tlen);
    auto pt = const_cast<icmp *>(this);
    pt->d.crc = dt.crc;
    std::memcpy(buf, &dt, sizeof(detail));
    bytes.insert(bytes.cbegin(), buf, buf + tlen);
    delete[] buf;
}

json icmp::to_json() const
{
    json j;
    j["type"] = type();
    if (type_dict.count(d.type) > 0) {
        j["icmp-type"] = type_dict.at(d.type).first;
        auto &code_dict = type_dict.at(d.type).second;
        if (code_dict.count(d.code) > 0) {
            j["desc"] = code_dict.at(d.code);
        }
    }
    j["id"] = d.u.s.id;
    j["serial-no"] = d.u.s.sn;
    size_t tlen = sizeof(detail) + extra.size();
    u_char *buf = new u_char[tlen];
    auto dt = hton(d);
    std::memcpy(buf, &dt, sizeof(detail));
    std::memcpy(buf + sizeof(detail), extra.data(), extra.size());
    j["checksum"] = calc_checksum(buf, tlen);
    delete[] buf;
    if (d.type == 0 || d.type == 8) {
        j["echo"] = std::string(reinterpret_cast<const char *>(extra.data()), extra.size());
    }
    return j;
}

std::string icmp::type() const { return Protocol_Type_ICMP; }

std::string icmp::succ_type() const { return Protocol_Type_Void; }

bool icmp::link_to(const protocol &rhs) const
{
    if (type() == rhs.type()) {
        auto p = dynamic_cast<const icmp &>(rhs);
        return d.u.s.id == p.d.u.s.id && d.u.s.sn == p.d.u.s.sn;
    }
    return false;
}

const icmp::detail &icmp::get_detail() const { return d; }

const std::vector<u_char> &icmp::get_extra() const { return extra; }

icmp::detail icmp::ntoh(const detail &d, bool reverse)
{
    detail dt = d;
    ntoh_cvt(dt.u.s.id, !reverse, s);
    ntoh_cvt(dt.u.s.sn, !reverse, s);
    return dt;
}

icmp::detail icmp::hton(const detail &d) { return ntoh(d, true); }
