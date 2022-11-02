#ifndef _RTSP_UTILS_H
#define _RTSP_UTILS_H
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <math.h>
#include <stdio.h>

#define VERSION "1.0"
#define PACKAGE "Alpha"
#define RTSP_VERSION "RTSP/1.0"

#define RTSP_BUFFERSIZE 4096
#define MAX_DESCR_LENGTH 4096

#define RTP_AVP			"RTP/AVP"
#define RTP_AVP_UDP		"RTP/AVP/UDP"
#define RTP_AVP_TCP		"RTP/AVP/TCP"
#define UNICAST			"unicast"


#define trace_point() 	do {printf("rtsp_tracepoint: %s,%s,%d\n",__FILE__,__FUNCTION__,__LINE__); } while(0)			//10728
//#define trace_sleep() 	do {printf("rtsp_tracesleep: %s,%d\n",__FILE__,__LINE__); sleep(1);} while(0)  //10728
//#define cz_trace		printf																		//10728
//#define cz_trip	 		do {printf("!CAUTION!-CZ_TRIP-: %s,%d\n",__FILE__,__LINE__); } while(0)		//10728

/************************error codes************************/

#define ERR_NOERROR          		0
#define ERR_GENERIC             	-1
#define ERR_NOT_FOUND       		-2
#define ERR_PARSE           		-3
#define ERR_ALLOC               	-4
#define ERR_INPUT_PARAM         	-5
#define ERR_NOT_SD          		-6
#define ERR_UNSUPPORTED_PT      	-7
#define ERR_EOF             		-8
#define ERR_FATAL                   -9
#define ERR_CONNECTION_CLOSE        -10

/************************RTSP message header************************/
#define HDR_VERSION					RTSP_VERSION
#define HDR_CONTENTTYPE 			"Content-Type"
#define HDR_CONTENTLENGTH 			"Content-Length"
#define HDR_ACCEPT 					"Accept"
#define HDR_ALLOW 					"Allow"
#define HDR_BLOCKSIZE 				"Blocksize"
#define HDR_DATE 					"Date"
#define HDR_REQUIRE 				"Require"
#define HDR_TRANSPORTREQUIRE 		"Transport-Require"
#define HDR_SEQUENCENO 				"SequenceNo"
#define HDR_CSEQ 					"CSeq"
#define HDR_STREAM 					"Stream"
#define HDR_SESSION 				"Session"
#define HDR_TRANSPORT 				"Transport"
#define HDR_RANGE 					"Range"
#define HDR_USER_AGENT 				"User-Agent"

/************************RTSP message method************************/
#define METHOD_MAXLEN 15
#define METHOD_NULL 				""
#define METHOD_OPTIONS 				"OPTIONS"
#define	METHOD_DESCRIBE 			"DESCRIBE"
#define METHOD_SETUP 				"SETUP"
#define METHOD_PLAY 				"PLAY"
#define METHOD_PAUSE 				"PAUSE"
#define METHOD_ANNOUNCE 			"ANNOUNCE"
#define METHOD_RECORD 				"RECORD"
#define METHOD_REDIRECT 			"REDIRECT"
#define METHOD_GET_PARAMETERS		"GET_PARAMETERS"
#define METHOD_SET_PARAMETER 		"SET_PARAMETER"
#define METHOD_TEARDOWN 			"TEARDOWN"

#define METHOD_NULL_ID				-1
#define METHOD_OPTIONS_ID			0
#define METHOD_DESCRIBE_ID			1
#define METHOD_SETUP_ID				2
#define METHOD_PLAY_ID				3
#define METHOD_PAUSE_ID				4
#define METHOD_ANNOUNCE_ID			5
#define METHOD_RECORD_ID			6
#define METHOD_REDIRECT_ID			7
#define METHOD_GET_PARAMETERS_ID	8
#define METHOD_SET_PARAMETER_ID		9
#define METHOD_TEARDOWN_ID			10

int RTSP_STATE_MACHINE(Client_Node *pClient_Node);
#endif
