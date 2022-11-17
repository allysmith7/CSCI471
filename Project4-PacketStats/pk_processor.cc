//
// Created by Phil Romig on 11/13/18.
//

#include "packetstats.h"
#include <net/ethernet.h>
#include <netinet/in.h>

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

  // ***********************************************************************
  // * Process the link layer header
  // ***********************************************************************
  //
  // Overlay "struct ether_header" onto the packet
  //
  // Extract the src/dst address, add them to results.
  // (use results->newSrcMac() and results->newDstMac())
  //
  //
  // Is it anything other Than Ethernet II? If so, record it and you are done.
  // length is the physical length of the packet (pkthdr->len)
  // - Record its existance using results->newOtherLink(length)
  //
  // Record it as Ethernet II
  // length is the physical length of the packet (pkthdr->len)
  // - Record its existance using results->newEthernet(length)
  struct ether_header *macHeader = (struct ether_header *)packet;

  if (ntohs(macHeader->ether_type) != ETHERTYPE_IP) {
    // not IPv4, skip
    return;
  }

  // ***********************************************************************
  // * Is it ARP?
  // ***********************************************************************
  //
  // Is it an ARP packet? If so, record it in results and you are done.
  // length is the physical length of the packet (pkthdr->len)
  // - Record its existance using results->newARP(length)
  //

  // ***********************************************************************
  // * Process the network layer header other than IPv4
  // ***********************************************************************
  //
  // Is it an IPv6 Packet?
  // length = Total packet length - Ethernet II header length
  // - Record its existance using results->newIPv6(length).
  //

  //
  // Is it anything other than IPv4, record it as other and you are done.
  // length = Total packet length - Ethernet II header length
  // - Record its existance using results->newOtherNetwork())
  //

  // ***********************************************************************
  // * Process IPv4 packets
  // ***********************************************************************
  //
  // If we are here, it must be IPv4, so overlay "struct ip" on the right
  // location. length = Total packet length - Ethernet II header length
  // - Record its existance using results->newIPv4(length)
  // - Record the src/dst addressed in the results class.

  // ***********************************************************************
  // * Process the Transport Layer
  // ***********************************************************************
  //
  // Is it TCP? Overlay the "struct tcphdr" in the correct location.
  // length = Total packet length - IPv4 header length - Ethernet II header
  // length
  // ** Don't forget that IPv4 headers can be different sizes.
  // - Record its existance using results->newTCP(length)
  // - Record src/dst ports (use results->newSrcTCP or newDstTCP.
  //   note you must store them in host order).
  // - Record SYN and/or FIN flags
  //   (use results->incrementSynCount(), results->incrementFinCount())

  // Is it UDP? Overlay the "struct udphdr" in the correct location.
  // length = Total packet length - IPv4 header length - Ethernet II header
  // length
  // ** Don't forget that IPv4 headers can be different sizes.
  // - Record its existance using results->newUDP(length)
  // - Record src/dst ports (must store them in host order).
  //   (use results->newSrcUDP()( and results->newDstUDP()
  //

  //
  // Is it ICMP,
  // length = Total packet length - IPv4 header length - Ethernet II header
  // length
  // ** Don't forget that IPv4 headers can be different sizes.
  // - Record its using results->newICMP(length);
  //

  //
  // Anything else, record as unknown transport.
  // length = Total packet length - IPv4 header length - Ethernet II header
  // length
  // ** Don't forget that IPv4 headers can be different sizes.
  // - Record its existence using results->newOtherTransport(length)
  //

  return;
}
