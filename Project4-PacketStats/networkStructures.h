//
// Created by Phil Romig on 10/31/18.
//

#pragma once

#include "packetstats.h"

#ifndef PACKETSTATS_NETWORKSTRUCTURES_H
#define PACKETSTATS_NETWORKSTRUCTURES_H

#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <net/ethernet.h>

#ifdef __linux__
    #include <netinet/ether.h>
#endif

#endif //PACKETSTATS_NETWORKSTRUCTURES_H
