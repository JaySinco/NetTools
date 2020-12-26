#pragma once
#include "protocol/type.h"
#include <map>
#include <tuple>
#include <mutex>

class port_table
{
public:
    using key_type = std::tuple<std::string, ip4, u_short>;

    using storage_t = std::map<key_type, std::string>;

    using image_cache_t =
        std::map<u_int, std::pair<std::string, std::chrono::system_clock::time_point>>;

    static void update();

    static void clear();

    static std::string lookup(const key_type &key);

private:
    static std::string pid_to_image(u_int pid);

    static storage_t tcp();

    static storage_t udp();

    static std::mutex lk_map;
    static storage_t map;

    static std::mutex lk_image;
    static image_cache_t image_cache;
};
