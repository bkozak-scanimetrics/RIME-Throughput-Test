/******************************************************************************
* Rime sink node tpt							      *
*									      *
* Implements a sink node as part of a throughput test of RIME. This node      *
* counts packets and prints throughput results through the debug output	      *
* interface.								      *
*									      *
* Author - Billy Kozak, Scanimetrics					      *
* Date - Tue. March. 10th, 2015						      *
*									      *
* Copyright Scanimetrics 2015						      *
******************************************************************************/

/******************************************************************************
*				    INCLUDES				      *
******************************************************************************/
#include "contiki.h"
#include "net/rime/rime.h"
#include "net/rime/collect.h"
#include "net/netstack.h"
#include "sys/clock.h"

#include "tpt.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
/******************************************************************************
*                                    TYPES                                    *
******************************************************************************/
struct tpt_dataTrack{
    //number of bytes recieved
    uint32_t dataCount;

    //number of whole packets recieved
    uint32_t packetCount;

    uint32_t misses;
    uint32_t errors;

    //time at which the first packet was recieved
    CCIF clock_time_t startTime;

    CCIF clock_time_t stopTime;

    bool txDone;
};
/******************************************************************************
*				    GLOBALS				      *
********************************** ********************************************/
static struct unicast_callbacks unicast_callbacks;
static struct unicast_conn uc;

static linkaddr_t clientAddr;
static struct pt tpt_handleState;
static struct tpt_dataTrack tracker;
/******************************************************************************
*			      FUNCTION PROTOTYPES			      *
******************************************************************************/
PROCESS(tpt_collect_process, "TPT collect process");
AUTOSTART_PROCESSES(&tpt_collect_process);

static void recv(struct unicast_conn *c, const linkaddr_t *from);
static PT_THREAD(sink_countData(const linkaddr_t *originator));
static inline bool tpt_isClient(const linkaddr_t *addr);
static void tpt_analyzePacket(void* dataptr,uint16_t len);
static void tpt_printTrack(void);
static void tpt_printError(
		uint32_t expVal,int errPos,uint8_t badByte,uint8_t expByte,
		uint16_t len
	);
/******************************************************************************
*			      FUNCTION DEFINITIONS			      *
******************************************************************************/
static void recv(struct unicast_conn *c, const linkaddr_t *from){
    sink_countData(from);
}
/*****************************************************************************/
static PT_THREAD(sink_countData(const linkaddr_t *originator)){
	PT_BEGIN(&tpt_handleState);

	while(1){

		memset(&tracker,0,sizeof(tracker));
		memcpy(&clientAddr,originator,sizeof(linkaddr_t));

		tracker.startTime = clock_time();

		printf("tpt from %d.%d\n",originator->u8[0],originator->u8[1]);

		while(1){
			if(tpt_isClient(originator)){
				tpt_analyzePacket(
						packetbuf_dataptr(),
						packetbuf_datalen()
					);

				if(tracker.txDone){
					break;
				}
			}
			PT_YIELD(&tpt_handleState);
		}

		tpt_printTrack();
		PT_YIELD(&tpt_handleState);
	}

	PT_END(&tpt_handleState);
}
/*****************************************************************************/
static inline bool tpt_isClient(const linkaddr_t *addr){
	return linkaddr_cmp(addr,&clientAddr);
}
/*****************************************************************************/
static void tpt_analyzePacket(void* dataptr,uint16_t len){

	//would have to add memory barrier to ensure that this happens first
	CCIF clock_time_t now = clock_time();

	if( len < 4 || (len%4) ){
		printf("bad packet len %d\n",len);
		tracker.errors += 1;
		return;
	}

	uint8_t* packet = dataptr;

	uint32_t packetVal = ((uint32_t)packet[0]);
	packetVal |= ((uint32_t)packet[1])<<8;
	packetVal |= ((uint32_t)packet[2])<<16;
	packetVal |= ((uint32_t)packet[3])<<24;

	int i;
	for(i = 4; i < len; i++){
		uint8_t expected = (packetVal>>(8*(i%4)))&0xFF;
		if(packet[i] != expected){
			tracker.errors += 1;
			tpt_printError(packetVal,i,packet[i],expected,len);
			return;
		}
	}

	tracker.dataCount += len;

	if(packetVal == TPT_STOP_VAL){
		tracker.stopTime = now;
		tracker.txDone = true;
	}
	else{
		tracker.misses += packetVal-(
				tracker.packetCount+tracker.misses
			);
	}

	tracker.packetCount += 1;

}
/*****************************************************************************/
static void tpt_printTrack(void){
	CCIF clock_time_t diff = tracker.stopTime - tracker.startTime;

	if( sizeof(unsigned) == 4 ){
		printf(
			"Got %u with %u errors and %u misses in %u ticks.\n",
			(unsigned)tracker.dataCount,
			(unsigned)tracker.errors,
			(unsigned)tracker.misses,
			(unsigned)diff
		);
	}
	else if (sizeof(unsigned long long) == 4){
		printf(
			"Got %llu with %llu errors and %llu misses in %llu "
			"ticks.\n",
			(unsigned long long)tracker.dataCount,
			(unsigned long long)tracker.errors,
			(unsigned long long)tracker.misses,
			(unsigned long long)diff
		);
	}
	else{
		printf("Error: un-accounted for integer length\n");
	}
}
/*****************************************************************************/
static void tpt_printError(
		uint32_t expVal,int errPos,uint8_t badByte,uint8_t expByte,
		uint16_t len
	){
	printf(
			"Error: val 0x%lx, len %d, bad byte 0x%x at %d,"
			"expected 0x%x\n",
			expVal,len,badByte,errPos,expByte
		);
}
/*****************************************************************************/
PROCESS_THREAD(tpt_collect_process, ev, data)
{
	PROCESS_BEGIN();

	//turn on sink to 100% duty cycle for maximum efficiency
	NETSTACK_MAC.off(1);
	unicast_callbacks.recv = recv;
	unicast_open(&uc, 146, &unicast_callbacks);

	uint8_t* laddr = linkaddr_node_addr.u8;
	uint16_t shortID = (laddr[1]<<0)|(laddr[0]<<8);


	if(shortID == SINK_NODEID) {
		printf("I am sink\n");
		//collect_set_sink(&tc, 1);
	}
	else{
		printf("bad sink id 0x%x\n",shortID);
	}

	while(1){
		PROCESS_YIELD();
	}

	PROCESS_END();
}
/*****************************************************************************/
