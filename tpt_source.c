/******************************************************************************
* Rime source node tpt                                                        *
*                                                                             *
* Implements a source node as part of a throughput test of RIME. This node    *
* sends packets to the sink at some predefined rate                           *
*                                                                             *
* Author - Billy Kozak, Scanimetrics                                          *
* Date - Tue. March. 10th, 2015                                               *
*                                                                             *
* Copyright Scanimetrics 2015                                                 *
******************************************************************************/

/******************************************************************************
*                                   INCLUDES                                  *
******************************************************************************/
#include "contiki.h"
#include "net/rime/rime.h"

#if CONTIKI_TARGET_SRF06_CC26XX
#include "srf06/button-sensor.h"
#else
#include "dev/button-sensor.h"
#endif

#include "tpt.h"

#include <stdio.h>
#include <string.h>

/******************************************************************************
*                                    DEFINES                                  *
******************************************************************************/
#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

#if (CONTIKI_TARGET_CC2538DK || CONTIKI_TARGET_SRF06_CC26XX)
#define TPT_START_BUTTON button_select_sensor
#else
#define TPT_START_BUTTON button_sensor
#endif
/******************************************************************************
*                                   GLOBALS                                   *
******************************************************************************/
static struct unicast_conn uc;
static struct unicast_callbacks unicast_callbacks;

static uint32_t tpt_sendCount;
static unsigned inTheAir;
static uint8_t  tpt_sendBuf[TPT_PACKET_LEN];
/******************************************************************************
*                             FUNCTION PROTOTYPES                             *
******************************************************************************/
PROCESS(tpt_unicast_process, "TPT unicast");
AUTOSTART_PROCESSES(&tpt_unicast_process);
static void recv_uc(struct unicast_conn *c, const linkaddr_t *from);
static void sent_uc(struct unicast_conn *uc, int status, int num_tx);
static void tpt_sendUnicast(struct unicast_conn *uc);
static void tpt_fillBuffer(uint32_t word);
static inline uint32_t tpt_fillValue(uint32_t sendCount);
static void tpt_startBurst(void);
/******************************************************************************
*                             FUNCTION DEFINITIONS                            *
******************************************************************************/
static void recv_uc(struct unicast_conn *c, const linkaddr_t *from){

}
/*****************************************************************************/
static void sent_uc(struct unicast_conn *uc, int status, int num_tx){

	if(tpt_sendCount < TPT_SEND_COUNT){
		tpt_fillBuffer(tpt_fillValue(tpt_sendCount));
		tpt_sendUnicast(uc);
	}
	else{
		inTheAir -= 1;
		if(!inTheAir){
			tpt_sendCount = 0;
			PRINTF("Test Done\n");
		}
	}
}
/*****************************************************************************/
static void tpt_startBurst(void){

	inTheAir = 0;

	int i;
	for(i = 0; i < TPT_BURST_COUNT; i++){
		tpt_fillBuffer(tpt_fillValue(tpt_sendCount));
		tpt_sendUnicast(&uc);
		inTheAir += 1;
	}
}
/*****************************************************************************/
static void tpt_sendUnicast(struct unicast_conn *uc){
	linkaddr_t addr;

	addr.u8[1] = (SINK_NODEID>>0)&0xFF;
	addr.u8[0] = (SINK_NODEID>>8)&0xFF;

	int i;
	for(i = 2; i < sizeof(addr.u8); i++){
		addr.u8[i] = 0;
	}

	packetbuf_copyfrom(tpt_sendBuf,TPT_PACKET_LEN);
	tpt_sendCount += 1;
	unicast_send(uc, &addr);
}
/*****************************************************************************/
static inline uint32_t tpt_fillValue(uint32_t sendCount){
	return (sendCount < (TPT_SEND_COUNT-1) ) ? sendCount : TPT_STOP_VAL;
}
/*****************************************************************************/
static void tpt_fillBuffer(uint32_t word){
	int i;
	for(i = 0; i < TPT_PACKET_LEN-(TPT_PACKET_LEN%4); i+=4){
		tpt_sendBuf[i+0] = (word>> 0)&0xFF;
		tpt_sendBuf[i+1] = (word>> 8)&0xFF;
		tpt_sendBuf[i+2] = (word>>16)&0xFF;
		tpt_sendBuf[i+3] = (word>>24)&0xFF;
	}
}
/*****************************************************************************/
PROCESS_THREAD(tpt_unicast_process, ev, data)
{
	PROCESS_EXITHANDLER(unicast_close(&uc);)

	PROCESS_BEGIN();

	unicast_callbacks.recv = recv_uc;
	unicast_callbacks.sent = sent_uc;

	unicast_open(&uc, 146, &unicast_callbacks);
	PRINTF(
		"TPT source node sending %d packets at %d bytes each\n",
		TPT_SEND_COUNT,TPT_PACKET_LEN
	);

	while(1){
		PROCESS_YIELD();
		if(ev == sensors_event && !tpt_sendCount){
			if(data == &(TPT_START_BUTTON)){
				PRINTF("Test Start\n");
				tpt_startBurst();
			}
		}
	}
	PROCESS_END();
}
/*****************************************************************************/
