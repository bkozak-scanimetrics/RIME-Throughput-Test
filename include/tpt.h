#ifndef _TPT_H_
#define _TPT_H_
/******************************************************************************
*                                   DEFINES                                   *
******************************************************************************/
#if !defined(CONTIKI_TARGET_EV_ADUCRF101MKXZ)
//max radio packet data size appears to 108 bytes on most platforms
#define TPT_PACKET_LEN  100
#else
#define TPT_PACKET_LEN  88
#endif

#define TPT_SEND_COUNT  1024
#define TPT_BURST_COUNT 1

#define TPT_STOP_VAL   0xFFFFFFFF

#define SINK_NODEID    0x0001

#ifdef CONTIKI_TARGET_CC2538DK
//let cc2538 pick it's own id
#define SOURCE_NODEID  0
#else
#define SOURCE_NODEID  0xAAAA
#endif
/******************************************************************************
*                                ERROR_CHECKING                               *
******************************************************************************/
#if (TPT_PACKET_LEN%4)
#error "Packet length must be divisible by 4"
#endif

#if (TPT_BURST_COUNT<1 || TPT_BURST_COUNT>TPT_SEND_COUNT)
#error "Absurd burst count value"
#endif

#endif //_TPT_H_
