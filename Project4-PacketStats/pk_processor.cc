//
// Created by Phil Romig on 11/13/18.
//

#include "packetstats.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/types.h>

const int TCP = 6;
const int UDP = 17;
const int ICMP = 1;

void hexdump(const u_char *src, uint n)
{
  for (int i = 0; i < n; i++)
  {
    printf("%x", *(src+i));
  }
}
uint64_t ether_ntouint(uint8_t *input)
{
  uint64_t result = 0;
  for (int i = 0; i < 6; i++)
  {
    result *= 16;
    result += input[i];
  }
  return result;
}
// ****************************************************************************
// * pk_processor()
// *  All of the work done by the program will be done here (or at least it
// *  it will originate here). The function will be called once for every
// *  packet in the savefile.
// ****************************************************************************
void pk_processor(u_char *user, const struct pcap_pkthdr *pkthdr, const u_char *packet)
{

  resultsC *results = (resultsC *)user;
  results->incrementTotalPacketCount();
  DEBUG << "Processing packet #" << results->packetCount() << ENDL;
  char s[256];
  memset(s, 0, 256);
  memcpy(s, ctime(&(pkthdr->ts.tv_sec)), strlen(ctime(&(pkthdr->ts.tv_sec))) - 1);
  TRACE << "\tPacket timestamp is " << s << ENDL;
  TRACE << "\tPacket capture length is " << pkthdr->caplen << ENDL;
  TRACE << "\tPacket physical length is " << pkthdr->len << ENDL;

  hexdump(packet, 85);
  exit(1);

  // ***********************************************************************
  // * Process the link layer header
  // ***********************************************************************
  //
  // Overlay "struct ether_header" onto the packet
  //
  // Extract the src/dst address, add them to results.
  // (use results->newSrcMac() and results->newDstMac())
  struct ether_header *macHeader = (struct ether_header *)packet;

  results->newSrcMac(ether_ntouint(macHeader->ether_shost));
  results->newDstMac(ether_ntouint(macHeader->ether_dhost));

  if (pkthdr->len < 42 || pkthdr->len > 1514) // TODO fix min length
  {
    // Is it anything other Than Ethernet II? If so, record it and you are done.
    // length is the physical length of the packet (pkthdr->len)
    // - Record its existance using results->newOtherLink(length)
    results->newOtherLink(pkthdr->len);
    return;
  }
  else
  {
    // Record it as Ethernet II
    // length is the physical length of the packet (pkthdr->len)
    // - Record its existance using results->newEthernet(length)
    results->newEthernet(pkthdr->len);
    if (ntohs(macHeader->ether_type) == ETHERTYPE_ARP)
    {
      // Is it an ARP packet? If so, record it in results and you are done.
      // length is the physical length of the packet (pkthdr->len)
      // - Record its existance using results->newARP(length)
      results->newARP(pkthdr->len);
      return;
    }
    else if (ntohs(macHeader->ether_type) == ETHERTYPE_IPV6)
    {
      // Is it an IPv6 Packet?
      // length = Total packet length - Ethernet II header length
      // - Record its existance using results->newIPv6(length).
      results->newIPv6(pkthdr->len - ETHER_HDR_LEN);
      return;
    }
    else if (ntohs(macHeader->ether_type) != ETHERTYPE_IP)
    {
      // Is it anything other than IPv4, record it as other and you are done.
      // length = Total packet length - Ethernet II header length
      // - Record its existance using results->newOtherNetwork())
      results->newOtherNetwork(pkthdr->len - ETHER_HDR_LEN);
      return;
    }
  }

  // ***********************************************************************
  // * Process IPv4 packets
  // ***********************************************************************
  //
  // If we are here, it must be IPv4, so overlay "struct ip" on the right
  // location. length = Total packet length - Ethernet II header length
  // - Record its existance using results->newIPv4(length)
  // - Record the src/dst addressed in the results class.
  struct ip *ipHeader = (struct ip *)packet + ETHER_HDR_LEN;
  results->newIPv4(pkthdr->len - ETHER_HDR_LEN);
  auto ipHeaderLen = ipHeader->ip_hl;

  hexdump(packet+ETHER_HDR_LEN, ipHeaderLen);
  exit(10);

  printf("%d\n", ipHeader->ip_v);

  // ***********************************************************************
  // * Process the Transport Layer
  // ***********************************************************************
  if (ipHeader->ip_p == TCP)
  {
    // Is it TCP? Overlay the "struct tcphdr" in the correct location.
    struct tcphdr *tcpHeader = (struct tcphdr *)(packet + ETHER_HDR_LEN + ipHeaderLen);
    // length = Total packet len - IPv4 header len - Ethernet II header len
    // ** Don't forget that IPv4 headers can be different sizes.
    // - Record its existance using results->newTCP(length)
    results->newTCP(pkthdr->len - ETHER_HDR_LEN - ipHeaderLen);
    // - Record src/dst ports (use results->newSrcTCP or newDstTCP.
    //   note you must store them in host order).
    // - Record SYN and/or FIN flags
    //   (use results->incrementSynCount(), results->incrementFinCount())
  }
  else if (ipHeader->ip_p == UDP)
  {
    // Is it UDP? Overlay the "struct udphdr" in the correct location.
    // length = Total packet len - IPv4 header len - Ethernet II header len
    // ** Don't forget that IPv4 headers can be different sizes.
    // - Record its existance using results->newUDP(length)
    // - Record src/dst ports (must store them in host order).
    //   (use results->newSrcUDP()( and results->newDstUDP()
  }
  else if (ipHeader->ip_p == ICMP)
  {
    // Is it ICMP?
    // length = Total packet length - IPv4 header length - Ethernet II header
    // length
    // ** Don't forget that IPv4 headers can be different sizes.
    // - Record its using results->newICMP(length);
  }
  else
  {
    // Anything else, record as unknown transport.
    // length = Total packet length - IPv4 header length - Ethernet II header
    // length
    // ** Don't forget that IPv4 headers can be different sizes.
    // - Record its existence using results->newOtherTransport(length)
  }

  return;
}
