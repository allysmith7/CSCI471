#include "includes.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <queue>
#include <sys/types.h>

// ***************************************************************************
// * ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
// *
// * These are the functions you need to fill in.
// ***************************************************************************

#define WINDOW_SIZE 10
// clang-format off
char ACK[20] = {'\x06', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x21'};
// clang-format on

// state variables
struct pkt sndpktA[WINDOW_SIZE];
struct pkt sndpktB;
uint base, nextseqnum, expectedseqnum;
double timeSent[WINDOW_SIZE], timeACKed[WINDOW_SIZE];
double estimatedRTT = 50.0, devRTT = 25.0;

// helper functions
struct pkt make_pkt(int sequenceNumber, int ackNumber, char data[20])
{
  struct pkt packet;

  // create packet with provided sequence number, etc.
  packet.seqnum = sequenceNumber;
  packet.acknum = ackNumber;
  memcpy(packet.payload, data, 20);
  // fill in checksum based on values provided
  packet.checksum = compute_checksum(packet);

  return packet;
}

void update_rtt_estimates(int sequenceNumber)
{
  int index = sequenceNumber % WINDOW_SIZE;
  TRACE << "UPDATE_RTT (" << simulation->getSimulatorClock() << "): seq: " << sequenceNumber << std::endl
        << "sent: " << timeSent[index] << std::endl
        << "ackd: " << timeACKed[index] << ENDL;

  // calculate the sample RTT for a given packet
  double sampleRTT = timeACKed[index] - timeSent[index];

  // update estimatedRTT and devRTT based on existing values, EWMA
  double p = 0.05;
  estimatedRTT = (1 - p) * estimatedRTT + p * sampleRTT;

  p = 0.1;
  devRTT = (1 - p) * devRTT + p * std::abs(sampleRTT - estimatedRTT);

  TRACE << "UPDATE_RTT new values - est: " << estimatedRTT << " dev: " << devRTT << ENDL;
}

uint get_timeout_len()
{
  // calculate timeout length based on stored estimatedRTT and devRTT
  double timeout = estimatedRTT + 2 * devRTT;
  DEBUG << "GET_TIMEOUT (" << simulation->getSimulatorClock() << ") currently " << timeout << ENDL;
  return timeout;
}

uint compute_checksum(struct pkt packet)
{
  // simple checksum, add each component together
  uint checksum = 0;
  checksum += packet.seqnum;
  checksum += packet.acknum;
  for (char c : packet.payload)
    checksum += (int)c;

  return checksum;
}

bool is_corrupted(struct pkt packet)
{
  uint calculated = compute_checksum(packet);
  return packet.checksum != calculated;
}

// ***************************************************************************
// * The following routine will be called once (only) before any other
// * entity A routines are called. You can use it to do any initialization
// ***************************************************************************
void A_init()
{
  base = 1;
  nextseqnum = 1;
}

// ***************************************************************************
// * The following routine will be called once (only) before any other
// * entity B routines are called. You can use it to do any initialization
// ***************************************************************************
void B_init()
{
  expectedseqnum = 1;
  sndpktB = make_pkt(nextseqnum, expectedseqnum, ACK);
}

// ***************************************************************************
// * Called from layer 5, passed the data to be sent to other side
// ***************************************************************************
bool rdt_sendA(struct msg message)
{
  INFO << "RDT_SEND_A (" << simulation->getSimulatorClock()
       << ") Layer 4 on side A has received a message from the application that should be sent to side B: " << message
       << ENDL;

  bool accepted;

  if (nextseqnum < base + WINDOW_SIZE)
  {
    // if there is room in the window, accept packet
    TRACE << "RDT_SEND_A: window has space, sending..." << ENDL;
    accepted = true;

    // create, store, and send packet
    struct pkt packet = make_pkt(nextseqnum, 0, message.data);

    sndpktA[nextseqnum % WINDOW_SIZE] = packet;
    timeSent[packet.seqnum % WINDOW_SIZE] = simulation->getSimulatorClock();
    simulation->udt_send(A, packet);

    // if at start of window, start timer
    if (base == nextseqnum)
    {
      uint rtt = get_timeout_len();
      simulation->start_timer(A, rtt);
    }
    nextseqnum++;
  }
  else
  {
    // if window is full, reject packet
    TRACE << "RDT_SEND_A: window was full, rejecting..." << ENDL;
    simulation->incRejections(A);
    accepted = false;
  }

  return accepted;
}

// ***************************************************************************
// * Called from layer 3, when a packet arrives for layer 4 on side A
// ***************************************************************************
void rdt_rcvA(struct pkt packet)
{
  INFO << "RDT_RCV_A (" << simulation->getSimulatorClock() << ") Layer 4 on side A has received a packet from side B "
       << packet << ENDL;
  simulation->incReceived(A);

  // for any non-corrupted packet received that has a non-negative RTT...
  if (!is_corrupted(packet))
  {
    int i = packet.acknum % WINDOW_SIZE;
    double rtt = simulation->getSimulatorClock() - timeSent[i];
    DEBUG << "RDT_RCV_A: rtt of " << rtt << ENDL;

    if (rtt >= 0)
    {
      // process acknowledgement
      timeACKed[i] = simulation->getSimulatorClock();
      base = packet.acknum + 1;
      simulation->stop_timer(A);
      if (base == nextseqnum)
      {
        // if at end of window, don't do anything special
        TRACE << "RDT_RCV_A: " << base << " == " << nextseqnum << ENDL;
      }
      else
      {
        // otherwise, update estimated RTT and start new timer if there isn't one running
        update_rtt_estimates(timeACKed[i]);
        TRACE << "RDT_RCV_A: " << base << " != " << nextseqnum << ENDL;
        simulation->start_timer(A, get_timeout_len());
      }
      return;
    }
  }
  // count any packets that are corrupted or have negative RTT
  simulation->incRejections(B);
}

// ***************************************************************************
// * Called from layer 5, passed the data to be sent to other side
// ***************************************************************************
bool rdt_sendB(struct msg message)
{
  INFO << "RDT_SEND_B (" << simulation->getSimulatorClock()
       << ") Layer 4 on side B has received a message from the application that should be sent to side A: " << message
       << ENDL;

  bool accepted = false;

  return (accepted);
}

// ***************************************************************************
// // called from layer 3, when a packet arrives for layer 4 on side B
// ***************************************************************************
void rdt_rcvB(struct pkt packet)
{
  INFO << "RTD_RCV_B (" << simulation->getSimulatorClock()
       << ") Layer 4 on side B has received a packet from layer 3 sent over the network from side A:" << packet
       << ",ex: " << expectedseqnum << ENDL;

  // for any non-corrupted, in-order packet...
  if (!is_corrupted(packet) && packet.seqnum == expectedseqnum)
  {
    DEBUG << "RTD_RCV_B: Packet was in-tact and in-order, passing up & send new ACK" << ENDL;
    // deliver to application layer
    struct msg message;
    memcpy(message.data, packet.payload, 20);
    simulation->deliver_data(B, message);

    // send ACK back
    sndpktB = make_pkt(0, packet.seqnum, ACK);
    simulation->udt_send(B, sndpktB);
    expectedseqnum++;
  }
  else
  {
    // reject packet, send old ACK
    DEBUG << "RTD_RCV_B: Packet was either corrupted or out of order, sending old ACK" << ENDL;
    simulation->udt_send(B, sndpktB);
    simulation->incRepetitions(B);
  }
}

// ***************************************************************************
// * Called when A's timer goes off
// ***************************************************************************
void A_timeout()
{
  INFO << "A_TIMEOUT (" << simulation->getSimulatorClock() << ") Side A's timer has gone off." << ENDL;
  TRACE << "A_TIMEOUT: Sending packets from " << base << " to " << nextseqnum << ENDL;

  // restart timer with a lofty estimate for timer
  simulation->stop_timer(A);
  simulation->start_timer(A, 2 * (estimatedRTT + 4 * devRTT));

  // resend all outstanding packets
  for (uint i = base; i < nextseqnum; i++)
  {
    simulation->udt_send(A, sndpktA[i % WINDOW_SIZE]);
    simulation->incRepetitions(A);
  }
}

// ***************************************************************************
// * Called when B's timer goes off
// ***************************************************************************
void B_timeout()
{
  INFO << "B_TIMEOUT (" << simulation->getSimulatorClock() << ") Side B's timer has gone off." << ENDL;
}
