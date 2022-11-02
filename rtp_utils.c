#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>

#include "rtsp_server.h"
#include "rtp_utils.h"

int RTPCreate(Client_Node *pClient_Node)
{
	struct sockaddr_in server_addr; 

	pClient_Node->message.rtp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(pClient_Node->message.rtp_socket_fd < 0)
	{
		printf("rtp-udp socket fail!\n");
		return -1;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family		= AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port		= 0;

	if(bind(pClient_Node->message.rtp_socket_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in)) < 0)
	{
		printf("rtp-udp bind fail!\n");
		return -1;
	}

	return 0;
}

int RTPDestroy(Client_Node *pClient_Node)
{
	if(pClient_Node->message.rtp_socket_fd > 0)
	{
		close(pClient_Node->message.rtp_socket_fd);
		pClient_Node->message.rtp_socket_fd = -1;
	}
	return 0; 
}

void *RTPHandle(void *pParam)
{
	char buf[10]={1,2,3,4,5,6,7,8,9};

	Client_Node *pClient_Node = (Client_Node*)pParam;
	if (pClient_Node == NULL)
	{
		goto out;
	}

	printf("<><><><>RTP handle start!<><><><>\n");
	while(pClient_Node->state == PLAYING_STATE && pClient_Node->message.rtp_socket_fd > 0)
	{
		sendto(pClient_Node->message.rtp_socket_fd, buf, sizeof(buf), 0, (struct sockaddr *)&pClient_Node->message.rtp_addr,sizeof(struct sockaddr_in));
		sleep(1);
	}

out:

	printf("<><><><>RTP handle exit!<><><><>\n");
	return NULL;
}


/*
static int SendNalu264(HndRtp hRtp, char *pNalBuf, int s32NalBufSize)
{
    return 0;
}

static int SendNalu711(HndRtp hRtp, char *buf, int bufsize)
{
    return 0;
}
*/
unsigned int RtpSend(unsigned int u32Rtp, char *pData, int s32DataSize, unsigned int u32TimeStamp)
{
    return 0;
}

