#pragma once
#include <iostream>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include "pcap.h"

std::ostream &operator<<(std::ostream &o, const sockaddr *s);
std::ostream &operator<<(std::ostream &o, const pcap_if_t *p);
