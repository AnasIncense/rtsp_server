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

int RTCPCreate(Client_Node *pClient_Node)
{
	int ret;
	int socket_fd;
	struct sockaddr_in server_addr; 
	socklen_t addr_len = sizeof(struct sockaddr_in);
	
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(socket_fd < 0)
	{
		printf("create udp socket fail!\n");
		return -1;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = 0;
	
	ret = bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in));
	if(ret < 0)
	{
		printf("udp socket bind fail!\n");
		return -1;
	}
	getsockname(socket_fd,(struct sockaddr *)&server_addr,&addr_len);
	printf("UDP Port = %d\n",htons(server_addr.sin_port));

	return 0;
}

void *RTCPHandle(void *pParam)
{
	
	return NULL;
}


