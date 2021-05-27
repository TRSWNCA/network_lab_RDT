#include <stdio.h>
#include <stdlib.h>

/* DEBUG mode controller */
#define DEBUG

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

#define BIDIRECTIONAL 0    /* change to 1 if you're doing extra credit */
/* and write a routine called B_output */

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg {
    char data[20];
};

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt {
    int seqnum;
    int acknum;
    int checksum;
    char payload[20];
};

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* function define start */
void init();

void generate_next_arrival();

void printevlist();

void stoptimer(int AorB);

void starttimer(int AorB, float increment);

void tolayer3(int AorB, struct pkt packet);

void tolayer5(int AorB, struct msg message);
/* function define end */

/* A global variables */
#define WINDOW_SIZE 8
#define SENDER_BUFFER_SIZE 50
int base;
int nextseqnum;
int nextbufferpos;
int A_timer_status; /* 0 -> timer not set yet; 1 -> timer already set */
struct pkt senderBuffer[SENDER_BUFFER_SIZE];

/* B global variables */
int expectedseqnum;
struct pkt B_sndpkt;

/* universal global utils */
#define MODNUM 19260817     /* for checksum */
#define BASENUM 114514      /* for checksum */
#define MAXTIME 20.0
#define DEFAULT_SEQ 1919810 /* for B_init */


/* help functions */

/*
 * make packet from message
 * para0: sequence number
 * para1: ack number
 * para2: packet address
 * para3: message
 */
void make_pkt(int num, int ack, struct pkt *packet, struct msg message) {
    int i;

    packet->seqnum = num;
    packet->acknum = ack;

    for (i = 0; i < 20; i++) {
        packet->payload[i] = message.data[i];
    }

    /* compute the checksum */
    int sum = 0;
    for (i = 0; i < 20; i++) {
        sum = (sum + ((num + ack + i + BASENUM) % MODNUM) * (int) message.data[i]) % MODNUM;
    }
    packet->checksum = sum;
}


/*
 * test checksum function
 * para: packet to test
 * return 1: packet corrupt
 * return 0: packet not corrupt
 */
int isCorrupt(struct pkt packet) {
    int i, sum = 0;
    for (i = 0; i < 20; i++) {
        sum = (sum + ((packet.seqnum + packet.acknum + i + BASENUM) % MODNUM) * (int) packet.payload[i]) % MODNUM;
    }

    if (sum == packet.checksum) { return 0; }
    else { return 1; }
}

/*
 * extract packet to message
 * para0: packet
 * para1: message address
 */
void extract(struct pkt packet, struct msg *message) {
    int i;

    for (i = 0; i < 20; i++) {
        message->data[i] = packet.payload[i];
    }
}


/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message) {
    int i;

#ifdef DEBUG
    printf("A receive message from above, ");
#endif

    if (nextbufferpos + 1 == base) {
        /* sender buffer full, drop message */
#ifdef DEBUG
        printf("sender buffer full, drop message\n");
#endif
        /* drop message, do not buffer */
    } else {
        /* sender buffer still available, buffer message first */
#ifdef DEBUG
        printf("sender buffer still available, buffer message first\n");
#endif
        /* buffer message */
        struct pkt bufferPkt;
        make_pkt(nextbufferpos, 0, &bufferPkt, message);
        senderBuffer[nextbufferpos].seqnum = bufferPkt.seqnum;
        senderBuffer[nextbufferpos].acknum = bufferPkt.acknum;
        senderBuffer[nextbufferpos].checksum = bufferPkt.checksum;
        for (i = 0; i < 20; i++) {
            senderBuffer[nextbufferpos].payload[i] = bufferPkt.payload[i];
        }
        /* increase nextbufferpos */
        nextbufferpos = (nextbufferpos + 1) % SENDER_BUFFER_SIZE;

        /* check if the buffered packet falls in window */
        if (((nextseqnum) % SENDER_BUFFER_SIZE) < ((base + WINDOW_SIZE) % SENDER_BUFFER_SIZE)) {
            /* buffered packet in sending window */
            if ((nextseqnum % SENDER_BUFFER_SIZE) != (nextbufferpos % SENDER_BUFFER_SIZE)) {
                /* not exceed buffered packet, able to send */
#ifdef DEBUG
                /* print current sending message */
                printf("Message contents: ");
                for (i = 0; i < 20; i++) {
                    printf("%c", message.data[i]);
                }
                printf("\n");
#endif
                /* udt_send */
                tolayer3(0, senderBuffer[nextseqnum]);
#ifdef DEBUG
                printf("packet%d has been sent to layer3\n", nextseqnum);
#endif
                if (base == nextseqnum) {
                    if (A_timer_status == 1) {
                        /* timer already set, stop it to avoid warning */
                        stoptimer(0);
                    }
                    starttimer(0, MAXTIME);
                    A_timer_status = 1;
                }
                /* increase nextseqnum */
                nextseqnum = (nextseqnum + 1) % SENDER_BUFFER_SIZE;
            } else {
                /* exceed buffered packet, unable to send */
            }
        } else {
            /* buffered packet out of sending window, do not send */
        }
    }

}

void B_output(struct msg message)  /* need be completed only for extra credit */
{
/*do nothing */
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet) {
    int isCorr = isCorrupt(packet);
    int ackNum = packet.acknum;

#ifdef DEBUG
    printf("A has received ACK: %d, corrupt: %d\n", ackNum, isCorr);
#endif

    if (isCorr == 0) {
        /* not corrupt */
        /* slide windows base */
        base = (ackNum + 1) % SENDER_BUFFER_SIZE;
#ifdef DEBUG
        printf("A slide window, base: %d, end: %d\n", base, (base + WINDOW_SIZE) % SENDER_BUFFER_SIZE);
#endif
        if (base == nextseqnum) {
            if (A_timer_status == 0) {
                /* timer not set yet, start it to avoid warning */
                starttimer(0, MAXTIME);
            }
            stoptimer(0);
            A_timer_status = 0;
        } else {
            if (A_timer_status == 1) {
                /* timer already set, stop it to avoid warning */
                stoptimer(0);
            }
            starttimer(0, MAXTIME);
            A_timer_status = 1;
        }
    } else {
        /* corrupt, do nothing */
#ifdef DEBUG
        printf("ACK corrupted\n");
#endif
    }

}

/* called when A's timer goes off */
void A_timerinterrupt() {
    /* time expire */
#ifdef DEBUG
    printf("A timer expired, resend packets in window and start timer\n");
#endif
    int i;
    /* start timer */
    starttimer(0, MAXTIME);
    /* resend all packets in window */
    for (i = base; i != nextseqnum; i = (i + 1) % SENDER_BUFFER_SIZE) {
        if ((i % SENDER_BUFFER_SIZE) != (nextbufferpos % SENDER_BUFFER_SIZE)) {
            /* not exceed buffered packet, able to resend */
            /* udt_send */
            tolayer3(0, senderBuffer[i]);
#ifdef DEBUG
            printf("A resend packet%d to layer3\n", i);
#endif
        }
    }
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init() {
    base = 1;
    nextseqnum = 1;
    nextbufferpos = 1;
    A_timer_status = 0;
}


/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(packet)
        struct pkt packet;
{
    int isCorr = isCorrupt(packet);
    int seqNum = packet.seqnum;

#ifdef DEBUG
    printf("B has received packet: %d, isCorr: %d\n", seqNum, isCorr);
#endif

    if ((isCorr == 0) && (seqNum == expectedseqnum)) {
        /* extract */
        struct msg message;
        extract(packet, &message);
        /* deliver_data */
        tolayer5(1, message);
#ifdef DEBUG
        printf("B deliver packet%d to layer5\n", seqNum);
#endif
        /* sndpkt=make_pkt(expectedseqnum,ACK,checksum) */
        make_pkt(DEFAULT_SEQ, expectedseqnum, &B_sndpkt, message);
        /* udt_send */
        tolayer3(1, B_sndpkt);
#ifdef DEBUG
        printf("B send ACK%d to layer3\n", expectedseqnum);
#endif
        /* increase expectedseqnum */
        expectedseqnum = (expectedseqnum + 1) % SENDER_BUFFER_SIZE;
    } else {
        /* default */
        /* udt_send */
        tolayer3(1, B_sndpkt);
    }

}

/* called when B's timer goes off */
void B_timerinterrupt() {
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init() {
    expectedseqnum = 1;
    /* meaningless message, only for make_pkt function */
    int i;
    struct msg message;
    for (i = 0; i < 20; i++) {
        message.data[i] = 'F';
    }
    /* sndpkt=make_pkt(0,ACK,checksum) */
    make_pkt(DEFAULT_SEQ, 0, &B_sndpkt, message);
}


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
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event {
    float evtime;           /* event time */
    int evtype;             /* event type code */
    int eventity;           /* entity where event occurs */
    struct pkt *pktptr;     /* ptr to packet (if any) assoc w/ this event */
    struct event *prev;
    struct event *next;
};
struct event *evlist = NULL;   /* the event list */

/* possible events: */
#define  TIMER_INTERRUPT 0
#define  FROM_LAYER5     1
#define  FROM_LAYER3     2

#define  OFF             0
#define  ON              1
#define   A    0
#define   B    1


int TRACE = 1;             /* for my debugging */
int nsim = 0;              /* number of messages from 5 to 4 so far */
int nsimmax = 0;           /* number of msgs to generate, then stop */
float time = 0.000;
float lossprob;            /* probability that a packet is dropped  */
float corruptprob;         /* probability that one bit is packet is flipped */
float lambda;              /* arrival rate of messages from layer 5 */
int ntolayer3;           /* number sent into layer 3 */
int nlost;               /* number lost in media */
int ncorrupt;              /* number corrupted by media*/

void insertevent(struct event *);


int main() {
    struct event *eventptr;
    struct msg msg2give;
    struct pkt pkt2give;

    int i, j;
    char c;

    init();
    A_init();
    B_init();

    while (1) {
        eventptr = evlist;            /* get next event to simulate */
        if (eventptr == NULL)
            goto terminate;
        evlist = evlist->next;        /* remove this event from event list */
        if (evlist != NULL)
            evlist->prev = NULL;
        if (TRACE >= 2) {
            printf("\nEVENT time: %f,", eventptr->evtime);
            printf("  type: %d", eventptr->evtype);
            if (eventptr->evtype == 0)
                printf(", timerinterrupt  ");
            else if (eventptr->evtype == 1)
                printf(", fromlayer5 ");
            else
                printf(", fromlayer3 ");
            printf(" entity: %d\n", eventptr->eventity);
        }
        time = eventptr->evtime;        /* update time to next event time */
        if (nsim == nsimmax)
            break;                        /* all done with simulation */
        if (eventptr->evtype == FROM_LAYER5) {
            generate_next_arrival();   /* set up future arrival */
            /* fill in msg to give with string of same letter */
            j = nsim % 26;
            for (i = 0; i < 20; i++)
                msg2give.data[i] = 97 + j;
            if (TRACE > 2) {
                printf("          MAINLOOP: data given to student: ");
                for (i = 0; i < 20; i++)
                    printf("%c", msg2give.data[i]);
                printf("\n");
            }
            nsim++;
            if (eventptr->eventity == A)
                A_output(msg2give);
            else
                B_output(msg2give);
        } else if (eventptr->evtype == FROM_LAYER3) {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i = 0; i < 20; i++)
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
            if (eventptr->eventity == A)      /* deliver packet by calling */
                A_input(pkt2give);            /* appropriate entity */
            else
                B_input(pkt2give);
            free(eventptr->pktptr);          /* free the memory for packet */
        } else if (eventptr->evtype == TIMER_INTERRUPT) {
            if (eventptr->eventity == A)
                A_timerinterrupt();
            else
                B_timerinterrupt();
        } else {
            printf("INTERNAL PANIC: unknown event type \n");
        }
        free(eventptr);
    }

    terminate:
    printf(" Simulator terminated at time %f\n after sending %d msgs from layer5\n", time, nsim);
}


void init()                         /* initialize the simulator */
{
    int i;
    float sum, avg;
    float jimsrand();


    printf("-----  Go Back N Network Simulator Version 1.1 -------- \n\n");
    printf("Enter the number of messages to simulate: ");
    scanf("%d", &nsimmax);
    printf("Enter  packet loss probability [enter 0.0 for no loss]:");
    scanf("%f", &lossprob);
    printf("Enter packet corruption probability [0.0 for no corruption]:");
    scanf("%f", &corruptprob);
    printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
    scanf("%f", &lambda);
    printf("Enter TRACE:");
    scanf("%d", &TRACE);

    srand(9999);              /* init random number generator */
    sum = 0.0;                /* test random number generator for students */
    for (i = 0; i < 1000; i++)
        sum = sum + jimsrand();    /* jimsrand() should be uniform in [0,1] */
    avg = sum / 1000.0;
    if (avg < 0.25 || avg > 0.75) {
        printf("It is likely that random number generation on your machine\n");
        printf("is different from what this emulator expects.  Please take\n");
        printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
        exit(0);
    }

    ntolayer3 = 0;
    nlost = 0;
    ncorrupt = 0;

    time = 0.0;                    /* initialize time to 0.0 */
    generate_next_arrival();     /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand() {
    double mmm = (double) RAND_MAX;//2147483647;   /* largest int  - MACHINE DEPENDENT!!!!!!!!   */
    float x;                   /* individual students may need to change mmm */
    x = rand() / mmm;            /* x should be uniform in [0,1] */
    return (x);
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

void generate_next_arrival() {
    double x, log(), ceil();
    struct event *evptr;
//   char *malloc();
    float ttime;
    int tempint;

    if (TRACE > 2)
        printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");

    x = lambda * jimsrand() * 2;  /* x is uniform on [0,2*lambda] */
    /* having mean of lambda        */
    evptr = (struct event *) malloc(sizeof(struct event));
    evptr->evtime = time + x;
    evptr->evtype = FROM_LAYER5;
    if (BIDIRECTIONAL && (jimsrand() > 0.5))
        evptr->eventity = B;
    else
        evptr->eventity = A;
    insertevent(evptr);
}


void insertevent(struct event *p) {
    struct event *q, *qold;

    if (TRACE > 2) {
        printf("            INSERTEVENT: time is %lf\n", time);
        printf("            INSERTEVENT: future time will be %lf\n", p->evtime);
    }
    q = evlist;     /* q points to header of list in which p struct inserted */
    if (q == NULL) {   /* list is empty */
        evlist = p;
        p->next = NULL;
        p->prev = NULL;
    } else {
        for (qold = q; q != NULL && p->evtime > q->evtime; q = q->next)
            qold = q;
        if (q == NULL) {   /* end of list */
            qold->next = p;
            p->prev = qold;
            p->next = NULL;
        } else if (q == evlist) { /* front of list */
            p->next = evlist;
            p->prev = NULL;
            p->next->prev = p;
            evlist = p;
        } else {     /* middle of list */
            p->next = q;
            p->prev = q->prev;
            q->prev->next = p;
            q->prev = p;
        }
    }
}

void printevlist() {
    struct event *q;
    int i;
    printf("--------------\nEvent List Follows:\n");
    for (q = evlist; q != NULL; q = q->next) {
        printf("Event time: %f, type: %d entity: %d\n", q->evtime, q->evtype, q->eventity);
    }
    printf("--------------\n");
}



/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
void stoptimer(int AorB)
/* A or B is trying to stop timer */
{
    struct event *q, *qold;

    if (TRACE > 2)
        printf("          STOP TIMER: stopping timer at %f\n", time);
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB)) {
            /* remove this event */
            if (q->next == NULL && q->prev == NULL)
                evlist = NULL;         /* remove first and only event on list */
            else if (q->next == NULL) /* end of list - there is one in front */
                q->prev->next = NULL;
            else if (q == evlist) { /* front of list - there must be event after */
                q->next->prev = NULL;
                evlist = q->next;
            } else {     /* middle of list */
                q->next->prev = q->prev;
                q->prev->next = q->next;
            }
            free(q);
            return;
        }
    printf("Warning: unable to cancel your timer. It wasn't running.\n");
}


void starttimer(int AorB, float increment)
/* A or B is trying to stop timer */
{

    struct event *q;
    struct event *evptr;
// char *malloc();

    if (TRACE > 2)
        printf("          START TIMER: starting timer at %f\n", time);
    /* be nice: check to see if timer is already started, if so, then  warn */
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB)) {
            printf("Warning: attempt to start a timer that is already started\n");
            return;
        }

/* create future event for when timer goes off */
    evptr = (struct event *) malloc(sizeof(struct event));
    evptr->evtime = time + increment;
    evptr->evtype = TIMER_INTERRUPT;
    evptr->eventity = AorB;
    insertevent(evptr);
}


/************************** TOLAYER3 ***************/
void tolayer3(int AorB, struct pkt packet)
/* A or B is trying to stop timer */
{
    struct pkt *mypktptr;
    struct event *evptr, *q;
// char *malloc();
    float lastime, x, jimsrand();
    int i;


    ntolayer3++;

    /* simulate losses: */
    if (jimsrand() < lossprob) {
        nlost++;
        if (TRACE > 0)
            printf("          TOLAYER3: packet being lost\n");
        return;
    }

/* make a copy of the packet student just gave me since he/she may decide */
/* to do something with the packet after we return back to him/her */
    mypktptr = (struct pkt *) malloc(sizeof(struct pkt));
    mypktptr->seqnum = packet.seqnum;
    mypktptr->acknum = packet.acknum;
    mypktptr->checksum = packet.checksum;
    for (i = 0; i < 20; i++)
        mypktptr->payload[i] = packet.payload[i];
    if (TRACE > 2) {
        printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
               mypktptr->acknum, mypktptr->checksum);
        for (i = 0; i < 20; i++)
            printf("%c", mypktptr->payload[i]);
        printf("\n");
    }

/* create future event for arrival of packet at the other side */
    evptr = (struct event *) malloc(sizeof(struct event));
    evptr->evtype = FROM_LAYER3;   /* packet will pop out from layer3 */
    evptr->eventity = (AorB + 1) % 2; /* event occurs at other entity */
    evptr->pktptr = mypktptr;       /* save ptr to my copy of packet */
/* finally, compute the arrival time of packet at the other end.
   medium can not reorder, so make sure packet arrives between 1 and 10
   time units after the latest arrival time of packets
   currently in the medium on their way to the destination */
    lastime = time;
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == FROM_LAYER3 && q->eventity == evptr->eventity))
            lastime = q->evtime;
    evptr->evtime = lastime + 1 + 9 * jimsrand();



    /* simulate corruption: */
    if (jimsrand() < corruptprob) {
        ncorrupt++;
        if ((x = jimsrand()) < .75)
            mypktptr->payload[0] = 'Z';   /* corrupt payload */
        else if (x < .875)
            mypktptr->seqnum = 999999;
        else
            mypktptr->acknum = 999999;
        if (TRACE > 0)
            printf("          TOLAYER3: packet being corrupted\n");
    }

    if (TRACE > 2)
        printf("          TOLAYER3: scheduling arrival on other side\n");
    insertevent(evptr);
}

void tolayer5(int AorB, struct msg message) {
    int i;
    if (TRACE > 2) {
        printf("          TOLAYER5: data received: ");
        for (i = 0; i < 20; i++)
            printf("%c", message.data[i]);
        printf("\n");
    }

}
