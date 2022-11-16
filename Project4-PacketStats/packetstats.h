//
// Created by Phil Romig on 10/30/18.
//

#pragma once

#ifndef PACKETSTATS_PACKETSTATS_H
#define PACKETSTATS_PACKETSTATS_H

// System include files
#include <unistd.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <unordered_set>
#include <limits.h>
#include <cstring>

// PCAP library
#include <pcap/pcap.h>

// Include files specific to this project.
#include "networkStructures.h"
#include "statisticsC.h"
#include "resultsC.h"

// Prototype for the packet processor, called once for each packet.
void pk_processor(u_char *, const struct pcap_pkthdr *, const u_char *);

// Boost free logging, NOT thread safe.
inline int LOGGING_LEVEL; //NOLINT
#define FATAL   if (LOGGING_LEVEL > -1) std::cout << "FATAL: "
#define ERROR   if (LOGGING_LEVEL > 0) std::cout  << "ERROR: "
#define WARNING if (LOGGING_LEVEL > 1) std::cout  << "WARNING: "
#define INFO    if (LOGGING_LEVEL > 2) std::cout  << "INFO: "
#define DEBUG   if (LOGGING_LEVEL > 3) std::cout  << "DEBUG: "
#define TRACE   if (LOGGING_LEVEL > 4) std::cout  << "TRACE: "

#define ENDL  " (" << __FILE__ << ":" << __LINE__ << ")" << std::endl

#endif //PACKETSTATS_PACKETSTATS_H
