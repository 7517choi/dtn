#include "contiki.h"
#include "net/rime/trickle.h"
#include "shared.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lib/random.h"

static process_event_t event_data_ready;
static struct etimer nodeSuccess;
int data = 5;
char *messageReceived;
char *sfReceived;
int sf = 7;
int n = 0;
int nOrig = 2;
int distValThis;
int distValThem;

/*---------------------------------------------------------------------------*/
PROCESS(beacon, "beacon");
PROCESS(packageSend, "Package sending");
AUTOSTART_PROCESSES(&beacon, &packageSend);
/*---------------------------------------------------------------------------*/
static void
trickle_recv(struct trickle_conn *c)
{ 
  printf("Node detected with SF: %s\r\n", (char *)packetbuf_dataptr());  
  sfReceived = (char *)packetbuf_dataptr();
  process_post(&packageSend, event_data_ready, &data);
}

static void
abc_recv(struct abc_conn *c)
{
  printf("Message received: %s\r\n", (char *)packetbuf_dataptr());
  if (n == 0) 
  {
    messageReceived = (char *)packetbuf_dataptr();
    n++;
  }
  else 
  {
    n++;
  }  

  if (1<= atoi(sfReceived) <= 3)
  {
    distValThem = 1;
  }
  else if (4<= atoi(sfReceived) <= 6)
  {
    distValThem = 3;
  }
  else if (7<= atoi(sfReceived) <= 9)
  {
    distValThem = 5;
  }
  else if (atoi(sfReceived) == 10)
  {
    distValThem = 7;
  }

  if (1<= sf <= 3)
  {
    distValThis = 1;
  }
  else if (4<= sf <= 6)
  {
    distValThis = 3;
  }
  else if (7<= sf <= 9)
  {
    distValThis = 5;
  }
  else if (sf == 10)
  {
    distValThis = 7;
  }

  int nCounter = n + nOrig;
  int distVal = distValThis + distValThem;
  nOrig = (nCounter/distVal)*distValThis;
 
}

const static struct trickle_callbacks trickle_call = {trickle_recv};
const static struct abc_callbacks abc_call = {abc_recv};
static struct trickle_conn trickle;
static struct abc_conn abc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(beacon, ev, data)
{  
  static struct etimer et;

  PROCESS_EXITHANDLER(trickle_close(&trickle);)
  PROCESS_BEGIN();

  trickle_open(&trickle, CLOCK_SECOND, 130, &trickle_call);
  
  while(1) {
    etimer_set(&et, CLOCK_SECOND);

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    packetbuf_copyfrom("7", 7);
    printf("This mule has SF %s\n", (char *)packetbuf_dataptr());
    trickle_send(&trickle);    
  }

  PROCESS_END();
}

PROCESS_THREAD(packageSend, ev, data)
{ 
  PROCESS_EXITHANDLER(abc_close(&abc);)
  PROCESS_BEGIN();
  abc_open(&abc, 128, &abc_call);
  PROCESS_WAIT_EVENT_UNTIL(ev == event_data_ready);
  while (nOrig != 0)
  {	
    packetbuf_copyfrom(messageReceived, 13);
    abc_send(&abc);
    nOrig --;
  }
  
  PROCESS_END();      
}

