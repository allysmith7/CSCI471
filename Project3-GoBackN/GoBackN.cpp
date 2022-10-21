#include "includes.h"

// ***************************************************************************
// * ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
// *
// * These are the functions you need to fill in.
// ***************************************************************************

// ***************************************************************************
// * The following routine will be called once (only) before any other
// * entity A routines are called. You can use it to do any initialization
// ***************************************************************************
void A_init() {
}

// ***************************************************************************
// * The following routine will be called once (only) before any other
// * entity B routines are called. You can use it to do any initialization
// ***************************************************************************
void B_init() {
}

// ***************************************************************************
// * Called from layer 5, passed the data to be sent to other side 
// ***************************************************************************
bool rdt_sendA(struct msg message) {
    INFO << "RDT_SEND_A: Layer 4 on side A has received a message from the application that should be sent to side B: "
              << message << ENDL;

    bool accepted = false;

    return (accepted);
}


// ***************************************************************************
// * Called from layer 3, when a packet arrives for layer 4 on side A
// ***************************************************************************
void rdt_rcvA(struct pkt packet) {
}


// ***************************************************************************
// * Called from layer 5, passed the data to be sent to other side
// ***************************************************************************
bool rdt_sendB(struct msg message) {
    INFO<< "RDT_SEND_B: Layer 4 on side B has received a message from the application that should be sent to side A: "
              << message << ENDL;

    bool accepted = false;

    return (accepted);
}


// ***************************************************************************
// // called from layer 3, when a packet arrives for layer 4 on side B 
// ***************************************************************************
void rdt_rcvB(struct pkt packet) {
    INFO << "RTD_RCV_B: Layer 4 on side B has received a packet from layer 3 sent over the network from side A:"
         << packet << ENDL;

}

// ***************************************************************************
// * Called when A's timer goes off 
// ***************************************************************************
void A_timeout() {
    INFO << "A_TIMEOUT: Side A's timer has gone off." << ENDL;
}

// ***************************************************************************
// * Called when B's timer goes off 
// ***************************************************************************
void B_timeout() {
    INFO << "B_TIMEOUT: Side B's timer has gone off." << ENDL;
}
