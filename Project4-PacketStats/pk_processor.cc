//
// Created by Phil Romig on 11/13/18.
//

#include "packetstats.h"
#include <cstdint>
#include <endian.h>

const int TCP = 6;
const int UDP = 17;
const int ICMP = 1;

std::string hexdump(const u_char *src, uint n)
{
  std::string result;
  for (int i = 0; i < n; i++)
  {
    if (i % 16 == 0)
      result += "\n";
    char str[n];
    sprintf(str, "%02x ", *(src + i));
    result += str;
  }
  return result;
}
uint64_t ether_ntouint(uint8_t *input)
{
  uint64_t result = 0;
  for (int i = 0; i < 6; i++)
  {
    result *= 256;
    result += input[5 - i];
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
  // TRACE << "\tContent: " << hexdump(packet, pkthdr->caplen) << ENDL;

  // ***********************************************************************
  // * Process the link layer header
  // ***********************************************************************
  struct ether_header *macHeader = (struct ether_header *)packet;
  uint64_t macSrc = ether_ntouint(macHeader->ether_shost);
  uint64_t macDest = ether_ntouint(macHeader->ether_dhost);
  results->newSrcMac(macSrc);
  results->newDstMac(macDest);
  TRACE << "\tSource MAC = " << ether_ntoa((const struct ether_addr *)&(macHeader->ether_shost)) << ENDL;
  TRACE << "\tDestination MAC = " << ether_ntoa((const struct ether_addr *)&(macHeader->ether_dhost)) << ENDL;
  TRACE << "\tEther Type = " << ntohs(macHeader->ether_type) << ENDL;

  // ***********************************************************************
  // ** If the value in ether_type is less than 1500 then the frame is
  // ** something other than Ethernet. We count tat as "other link" and
  // ** and we are done.
  // ***********************************************************************
  if (ntohs(macHeader->ether_type) <= 1500)
  {
    TRACE << "\tPacket is not Ethernet II" << ENDL;
    results->newOtherLink(pkthdr->len);
    return;
  }

  // ***********************************************************************
  // * Now we know the frame is Ethernet II
  // ***********************************************************************
  TRACE << "\tPacket is Ethernet II" << ENDL;
  results->newEthernet(pkthdr->len);

  if (ntohs(macHeader->ether_type) == ETHERTYPE_ARP)
  {
    TRACE << "\tPacket is ARP" << ENDL;
    results->newARP(pkthdr->len);
    return;
  }

  // ***********************************************************************
  // *****************   Process he Network Layer   ************************
  // ***********************************************************************

  // ***********************************************************************
  // * First, identify the IPv6 and Other Network using the type field
  // * of the Ethernet frame.
  // ***********************************************************************
  int networkPacketLength = pkthdr->len - 14;
  if (ntohs(macHeader->ether_type) == ETHERTYPE_IPV6)
  {
    TRACE << "\t\tPacket is IPv6, length is " << networkPacketLength << ENDL;
    results->newIPv6(networkPacketLength);
    return;
  }

  if (ntohs(macHeader->ether_type) != ETHERTYPE_IP)
  {
    TRACE << "\t\tPacket has an unrecognized ETHERTYPE" << ntohs(macHeader->ether_type) << ENDL;
    results->newOtherNetwork(networkPacketLength);
    return;
  }

  // ***********************************************************************
  // * Now we know it MUST be an IPv4 Packet
  // ***********************************************************************
  TRACE << "\t\tPacket is IPv4, length is " << networkPacketLength << ENDL;
  results->newIPv4(networkPacketLength);
  struct ip *ipHeader = (struct ip *)(packet + 14);

  // multiplied by 4 since it is 32bits wide, 8 bits per byte
  int ipHeaderLen = ipHeader->ip_hl * 4;
  // TRACE << "IP Content: " << hexdump((const u_char *)ipHeader, ipHeaderLen) << ENDL;
  DEBUG << "IP Header Length: " << ipHeaderLen << ENDL;

  results->newSrcIPv4(ipHeader->ip_src.s_addr);
  results->newDstIPv4(ipHeader->ip_dst.s_addr);

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
    results->newSrcTCP(be16toh(tcpHeader->source));
    results->newDstTCP(be16toh(tcpHeader->dest));
    // - Record SYN and/or FIN flags
    //   (use results->incrementSynCount(), results->incrementFinCount())
    if ((tcpHeader->th_flags & TH_FIN) == TH_FIN)
      results->incrementFinCount();
    if ((tcpHeader->th_flags & TH_SYN) == TH_SYN)
      results->incrementSynCount();
  }
  else if (ipHeader->ip_p == UDP)
  {
    // Is it UDP? Overlay the "struct udphdr" in the correct location.
    struct udphdr *udpHeader = (struct udphdr *)(packet + ETHER_HDR_LEN + ipHeaderLen);
    // length = Total packet len - IPv4 header len - Ethernet II header len
    // ** Don't forget that IPv4 headers can be different sizes.
    // - Record its existance using results->newUDP(length)
    results->newUDP(pkthdr->len - ETHER_HDR_LEN - ipHeaderLen);
    // - Record src/dst ports (must store them in host order).
    results->newSrcUDP(be16toh(udpHeader->source));
    results->newDstUDP(be16toh(udpHeader->dest));
    //   (use results->newSrcUDP()( and results->newDstUDP()
  }
  else if (ipHeader->ip_p == ICMP)
  {
    // Is it ICMP?
    // length = Total packet length - IPv4 header length - Ethernet II header
    // length
    // ** Don't forget that IPv4 headers can be different sizes.
    // - Record its using results->newICMP(length);
    results->newICMP(pkthdr->len - ETHER_HDR_LEN - ipHeaderLen);
  }
  else
  {
    // Anything else, record as unknown transport.
    // length = Total packet length - IPv4 header length - Ethernet II header
    // length
    // ** Don't forget that IPv4 headers can be different sizes.
    // - Record its existence using results->newOtherTransport(length)
    results->newOtherTransport(pkthdr->len - ETHER_HDR_LEN - ipHeaderLen);
  }

  return;
}
