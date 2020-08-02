#pragma once
#include <iostream>
#include <iomanip>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include "pcap.h"

std::ostream &operator<<(std::ostream &out, const sockaddr *addr);
std::ostream &operator<<(std::ostream &out, const pcap_if_t *dev);
std::ostream &print_packet(std::ostream &out, const pcap_pkthdr *header, const u_char *data);
