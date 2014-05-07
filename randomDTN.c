#include "contiki.h"
#include "net/rime/trickle.h"
#include "shared.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lib/random.h"

static process_event_t event_data_ready;
static struct etimer nodeSuccess;
char *messageReceived = "Test Message";
int n = 1;
/*---------------------------------------------------------------------------*/
PROCESS(beacon, "beacon");
PROCESS(packageSend, "Package sending");
AUTOSTART_PROCESSES(&beacon, &packageSend);
/*---------------------------------------------------------------------------*/
static void
trickle_recv(struct trickle_conn *c)
{ 
  printf("Node detected: %s\r\n", (char *)packetbuf_dataptr());
  
  int data = 5;
  process_post(&packageSend, event_data_ready, &data); 
}

static void
abc_recv(struct abc_conn *c)
{
  printf("Message received: %s\r\n", (char *)packetbuf_dataptr());
  
  if (n == 0) 
  {
    messageReceived = (char *)packetbuf_dataptr();
  }
  else
  {
    printf("Already have\n");
  }
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
    etimer_set(&et, CLOCK_SECOND * 2 + random_rand() % (CLOCK_SECOND * 2));

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    packetbuf_copyfrom("Node x", 7);
    printf("Transmission request from %s\n", (char *)packetbuf_dataptr());
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
  if (n == 1)
  {   
    if ((random_rand()%2) == 2)
      {
        packetbuf_copyfrom(messageReceived, 13);
        abc_send(&abc);
        n = 0;
      }
  }
  PROCESS_END();      
}

