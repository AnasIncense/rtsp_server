#ifndef _RTP_UTILS_H
#define _RTP_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>


#define MAX_RTP_PKT_LENGTH     1400

#define H264                    96
#define G711					97

typedef enum
{
	_h264		= 0x100,
	_h264nalu,
	_mjpeg,
	_g711		= 0x200,
}EmRtpPayload;
enum H264_FRAME_TYPE {FRAME_TYPE_I, FRAME_TYPE_P, FRAME_TYPE_B};


typedef struct
{
    /**//* byte 0 */
    unsigned char u4CSrcLen:4;      /**//* expect 0 */
    unsigned char u1Externsion:1;   /**//* expect 1, see RTP_OP below */
    unsigned char u1Padding:1;      /**//* expect 0 */
    unsigned char u2Version:2;      /**//* expect 2 */
    /**//* byte 1 */
    unsigned char u7Payload:7;      /**//* RTP_PAYLOAD_RTSP */
    unsigned char u1Marker:1;       /**//* expect 1 */
    /**//* bytes 2, 3 */
    unsigned short u16SeqNum;
    /**//* bytes 4-7 */
    unsigned long long u32TimeStamp;
    /**//* bytes 8-11 */
    unsigned long u32SSrc;          /**//* stream number is used here. */
} StRtpFixedHdr;

typedef struct
{
    //byte 0
    unsigned char u5Type:5;
    unsigned char u2Nri:2;
    unsigned char u1F:1;
} StNaluHdr; /**/ /* 1 BYTES */

typedef struct
{
    //byte 0
    unsigned char u5Type:5;
    unsigned char u2Nri:2;
    unsigned char u1F:1;
} StFuIndicator; /**/ /* 1 BYTES */

typedef struct
{
    //byte 0
    unsigned char u5Type:5;
    unsigned char u1R:1;
    unsigned char u1E:1;
    unsigned char u1S:1;
} StFuHdr; /**/ /* 1 BYTES */

typedef struct _tagStRtpHandle
{
    int                 s32Sock;
    struct sockaddr_in  stServAddr;
    unsigned short      u16SeqNum;
    unsigned long long        u32TimeStampInc;
    unsigned long long        u32TimeStampCurr;
    unsigned long long      u32CurrTime;
    unsigned long long      u32PrevTime;
    unsigned int        u32SSrc;
    StRtpFixedHdr       *pRtpFixedHdr;
    StNaluHdr           *pNaluHdr;
    StFuIndicator       *pFuInd;
    StFuHdr             *pFuHdr;
    EmRtpPayload        emPayload;
#ifdef SAVE_NALU
    FILE                *pNaluFile;
#endif
} StRtpObj, *HndRtp;

int RTPCreate(Client_Node *pClient_Node);
int RTPDestroy(Client_Node *pClient_Node);
void *RTPHandle(void *pParam);

#endif