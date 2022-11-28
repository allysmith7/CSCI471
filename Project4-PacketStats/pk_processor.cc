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
  //
  // Overlay "struct ether_header" onto the packet
  //
  // Extract the src/dst address, add them to results.
  // (use results->newSrcMac() and results->newDstMac())
  struct ether_header *macHeader = (struct ether_header *)packet;

  results->newSrcMac(ether_ntouint(macHeader->ether_shost));
  results->newDstMac(ether_ntouint(macHeader->ether_dhost));

  TRACE << "\tSource Mac: " << hexdump((const u_char *)&macHeader->ether_shost, 6) << ENDL;
  TRACE << "\tDest Mac: " << hexdump((const u_char *)&macHeader->ether_dhost, 6) << ENDL;

  // Record it as Ethernet II
  // length is the physical length of the packet (pkthdr->len)
  // - Record its existance using results->newEthernet(length)
  if (ntohs(macHeader->ether_type) == ETHERTYPE_ARP)
  {
    // Is it an ARP packet? If so, record it in results and you are done.
    // length is the physical length of the packet (pkthdr->len)
    // - Record its existance using results->newARP(length)
    results->newEthernet(pkthdr->len);
    results->newARP(pkthdr->len);
    return;
  }
  else if (ntohs(macHeader->ether_type) == ETHERTYPE_IPV6)
  {
    // Is it an IPv6 Packet?
    // length = Total packet length - Ethernet II header length
    // - Record its existance using results->newIPv6(length).
    results->newEthernet(pkthdr->len);
    results->newIPv6(pkthdr->len - ETHER_HDR_LEN);
    return;
  }
  else if (ntohs(macHeader->ether_type) != ETHERTYPE_IP)
  {
    // Is it anything other than IPv4, record it as other and you are done.
    // length = Total packet length - Ethernet II header length
    // - Record its existance using results->newOtherNetwork())
    results->newOtherLink(pkthdr->len);
    return;
  }
    results->newEthernet(pkthdr->len);
  // ***********************************************************************
  // * Process IPv4 packets
  // ***********************************************************************
  //
  // If we are here, it must be IPv4, so overlay "struct ip" on the right
  // location. length = Total packet length - Ethernet II header length
  // - Record its existance using results->newIPv4(length)
  // - Record the src/dst addressed in the results class.
  struct ip *ipHeader = (struct ip *)(packet + ETHER_HDR_LEN);

  // multiplied by 4 since it is 32bits wide, 8 bits per byte
  int ipHeaderLen = ipHeader->ip_hl * 4;
  results->newIPv4(pkthdr->len - ETHER_HDR_LEN);

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
