/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose

   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 0 /* change to 1 if you're doing extra credit */
/* and write a routine called B_output */

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg
{
  char data[20];
};

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt
{
  int seqnum;
  int acknum;
  int checksum;
  char payload[20];
};

/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
  - emulates the tranmission and delivery (possibly with bit-level corruption
    and packet loss) of packets across the layer 3/4 interface
  - handles the starting/stopping of a timer, and generates timer
    interrupts (resulting in calling students timer handler).
  - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should not
have to, and you defeinitely should not have to modify
******************************************************************/

struct event
{
  double evtime;      /* event time */
  int evtype;         /* event type code */
  int eventity;       /* entity where event occurs */
  struct pkt *pktptr; /* ptr to packet (if any) assoc w/ this event */
  struct event *prev;
  struct event *next;
};

/* possible events: */
#define TIMER_INTERRUPT 0
#define FROM_LAYER5 1
#define FROM_LAYER3 2
static const char *EVENT_NAMES[] = {"TIMER_INTERRUPT", "FROM_LAYER5", "FROM_LAYER3"};

#define A 0
#define B 1
static const char *SIDE_NAMES[] = {"A", "B"};

class simulator
{
private:
  long nsim;    /* number of messages from 5 to 4 so far */
  long nsimmax; /* number of msgs to generate, then stop */
  double kr_time;
  double lossprob;         /* probability that a packet is dropped  */
  double corruptprob;      /* probability that one bit is packet is flipped */
  double lambda;           /* arrival rate of messages from layer 5 */
  int ntolayer3;           /* number sent into layer 3 */
  int nlost;               /* number lost in media */
  int ncorrupt;            /* number corrupted by media*/
  struct event *evlist;    /* the event list */
  int messagesReceived[2]; /* The number of messages received by the program */
  int messagesSent[2];     /* The number of messages sent by the program */
  int messagesRejected[2]; /* The number of messages rejected by the program */
  int messagesRepeated[2]; /* The number of messages repeated by the program */

  double jimsrand();
  void generate_next_arrival();
  void insertevent(struct event *p);
  void reportPacketsInFlight(int AorB);
  void printevlist();

public:
  simulator(long n, double l, double c, double t);
  void go();
  double getSimulatorClock();
  void stop_timer(int AorB);
  void start_timer(int AorB, float increment);
  void udt_send(int AorB, struct pkt packet);
  void deliver_data(int AorB, struct msg message);
  void incReceived(int AorB);
  void incRejections(int AorB);
  void incRepetitions(int AorB);
};
