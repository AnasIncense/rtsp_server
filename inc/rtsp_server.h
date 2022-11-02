#ifndef RTSP_SERVER_H
#define RTSP_SERVER_H

#define TRUE	1
#define FALSE   0

#define RTSP_SERVER_PORT				8554
#define RTSP_SERVER_URL					"rtsp://%s:%d/stream%s"
#define RTSP_SERVER_MAXIMUM_CLIENTS		12

#define INIT_STATE      			0
#define READY_STATE     			1
#define PLAYING_STATE				2

typedef struct _RTSP_Message
{
	int CSeq;	
	int method_id;
    int rtp_socket_fd;
    int rtcp_socket_fd;

	pthread_t tidp;

	char url[64];
	char session[8];
	char buf[1024];

	struct sockaddr_in rtp_addr;
    struct sockaddr_in rtcp_addr;
}RTSP_Message;

typedef struct _Client_List
{
	int alive;
	int state;
    int socket_fd;
	
	pthread_t tidp;

	struct sockaddr_in client_addr;//TCP
    struct _RTSP_Message message;
	struct _Client_List *next;
}Client_List,Client_Node;

typedef struct RTSP_SERVER_INFO
{
	char rtsp_url[1024];
	struct sockaddr serverAddr;
}Server_Info;

int RTSPServer(void *pParam);

#endif
