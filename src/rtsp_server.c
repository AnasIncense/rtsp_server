#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h> 
#include <sys/ipc.h> 
#include <sys/msg.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/sockios.h>
#include <netinet/in.h> 
#include <arpa/inet.h>

#include "rtsp_server.h"
#include "rtsp_utils.h"

void signal_handler(int signo)
{
   exit(-1);
}

Client_Node *CreateClientList(void)
{
	Client_Node *pClient_Node = NULL;
	
	pClient_Node = (Client_Node *)calloc(1,sizeof(Client_Node));
	memset(pClient_Node,0,sizeof(Client_Node));
	pClient_Node->alive = TRUE;
	pClient_Node->state = INIT_STATE;
	return pClient_Node;
}

Client_Node *InsertClientNode(Client_List **ppClient_List)
{
	int client = 0;
	Client_Node *pClient_Node = *ppClient_List;
	while(pClient_Node->next != NULL)
	{
		pClient_Node = pClient_Node->next;
		client++;
		if(client > RTSP_SERVER_MAXIMUM_CLIENTS)
		{
			fprintf(stderr, "maximum client %d! %s,%i\n", RTSP_SERVER_MAXIMUM_CLIENTS, __FILE__, __LINE__);
			return NULL;
		}
	}
	pClient_Node->next  = (Client_Node *)calloc(1,sizeof(Client_Node));
    if (pClient_Node->next  == NULL)
    {
		fprintf(stderr, "error calloc %s,%i\n", __FILE__, __LINE__);
		return NULL;
    }
	pClient_Node->next->next = NULL;

	return pClient_Node->next;
}

Client_Node *DeleteClientNode(Client_Node *pClient_Node)
{
	return NULL;
}

Client_Node *RefreshClientList(Client_List *pClient_List)
{
	Client_Node *pNode 		  = pClient_List;
	Client_Node *pClient_Node = pClient_List;
	
	while(pClient_Node != NULL)
	{
		if(pClient_Node->alive == FALSE)
		{
			pNode->next = pClient_Node->next;
			free(pClient_Node);			
			pClient_Node = pNode->next;
		}
		else
		{
			pNode = pClient_Node;
			pClient_Node = pClient_Node->next;
		}
	}

	return pNode;
}

int DestroyClientList(Client_List *pClient_List)
{
	Client_Node *pClient_Node = pClient_List;
	if(pClient_Node == NULL)
	{
		return -1;
	}
	return 0;
}

void *RTSPHandle(void *pParam)
{
	int ret;	
	struct timeval time;
	fd_set rset,wset;
	pthread_detach(pthread_self());

	Client_Node *pClient_Node = (Client_Node*)pParam;
	if (pClient_Node == NULL)
	{
		goto out;
	}

	printf("<><><><>RTSP handle start!<><><><>\n");
	while(1)
	{
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		time.tv_sec		= 0;
		time.tv_usec	= 100000;

		FD_SET(pClient_Node->socket_fd,&rset);

		if (select(pClient_Node->socket_fd+1,&rset,0,0,&time)<0)
		{
			fprintf(stderr,"select error %s %d\n", __FILE__, __LINE__);
			//send_reply(500, NULL, rtsp);
			goto out;
		}

		if (FD_ISSET(pClient_Node->socket_fd,&rset))
		{
			memset(pClient_Node->message.buf,0,sizeof(pClient_Node->message.buf));

			ret = recv(pClient_Node->socket_fd, pClient_Node->message.buf, sizeof(pClient_Node->message.buf), 0);		
			if (ret < 0)
			{
				fprintf(stderr,"read() error %s %d\n", __FILE__, __LINE__);
				//send_reply(500, NULL, rtsp);
			}
			else if (ret == 0)
			{
				fprintf(stderr,"client disconnect! %s %d\n", __FILE__, __LINE__);
				goto out;
			}
			if (ret > 0)
			{
				ret = RTSP_STATE_MACHINE(pClient_Node);
				if(ret < 0)
				{
					fprintf(stderr,"state machine end! %s %d\n", __FILE__, __LINE__);
					goto out;
				}
			}
		}
	}

out:
	if(pClient_Node->message.rtp_socket_fd > 0)
	{
		close(pClient_Node->message.rtp_socket_fd);
		pClient_Node->message.rtp_socket_fd = -1;
	}

	pthread_join(pClient_Node->message.tidp, NULL);

	if(pClient_Node->socket_fd > 0)
	{
		close(pClient_Node->socket_fd);
		pClient_Node->socket_fd = -1;
	}

	pClient_Node->alive = FALSE;
	printf("<><><><>RTSP handle exit!<><><><>\n");
	return NULL;
}

int RTSPServer(void *pParam)
{
    int ret;
    int optval = 1;
    int iocval = 1;
    int server_socket_fd;
    int client_socket_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family 		= AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port 		= htons(RTSP_SERVER_PORT); 

	server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket_fd < 0)
	{
		printf("socket error!\n");
		goto out;
    }

	ret = ioctl(server_socket_fd, FIONBIO, &iocval);
    if(ret < 0)
    {
        fprintf(stderr, "ioctl() error\n");
		goto out;
    }
	
	ret = setsockopt(server_socket_fd ,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(int));
    if(ret < 0)
	{
		fprintf(stderr,"setsockopt error!\n");
		goto out;
    }

    ret = bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
    if(ret < 0)
    {
		fprintf(stderr,"bind error!\n");
		goto out;
    }

    ret = listen(server_socket_fd, RTSP_SERVER_MAXIMUM_CLIENTS); 
    if(ret < 0)
    {
		fprintf(stderr,"listen error!\n");
		goto out;
    }

	Client_List *pClient_List = NULL;//must point to the head!
	Client_Node *pClient_Node = NULL;
	pClient_List = CreateClientList();
	if(pClient_List == NULL)
	{
		fprintf(stderr,"Client List malloc fail!\n");
		goto out;
	}
	
	printf("\n<><><><>RTSP server start!<><><><>\n");
	while(1)
    {
		client_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, &addrlen);
		if(client_socket_fd >= 0)
		{		
			printf("client %d %s:%d connected!\n",client_socket_fd,inet_ntoa(client_addr.sin_addr),htons(client_addr.sin_port));
			pClient_Node = InsertClientNode(&pClient_List);
			if(pClient_Node == NULL)
			{
				printf("client node add fail!\n");
				continue;
			}
			pClient_Node->alive 	= TRUE;			
			pClient_Node->state 	= INIT_STATE;
			pClient_Node->socket_fd = client_socket_fd;
			memcpy(&pClient_Node->client_addr,&client_addr,sizeof(struct sockaddr_in));
			pClient_Node->message.CSeq = -1;
			pthread_create(&pClient_Node->tidp, NULL, RTSPHandle, pClient_Node);
		}
		pClient_Node = NULL;
		RefreshClientList(pClient_List);
		usleep(10);
    }

	DestroyClientList(pClient_List);
	close(server_socket_fd);
out:

	printf("\n<><><><>RTSP server exit!<><><><>\n");
	
	return 0;
}
