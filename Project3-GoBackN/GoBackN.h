
// ***********************************************************
// * Any additional include files should be added here.
// ***********************************************************
#include <vector>

// ***********************************************************
// * Any functions you want to add should be included here.
// ***********************************************************
struct pkt make_pkt(int sequenceNumber, int ackNumber, char data[20]);
int compute_checksum(struct pkt packet);