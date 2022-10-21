
// ***********************************************************
// * Any additional include files should be added here.
// ***********************************************************

// ***********************************************************
// * Any functions you want to add should be included here.
// ***********************************************************
struct pkt make_pkt(int sequenceNumber, char data[20]);
int computeChecksum(struct pkt packet);