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
#include "rtp_utils.h"

struct
{
	const char *method;
	int method_id;
}HDR_METHOD[] ={{METHOD_OPTIONS, METHOD_OPTIONS_ID},{METHOD_DESCRIBE, METHOD_DESCRIBE_ID},{METHOD_SETUP, METHOD_SETUP_ID},{METHOD_PLAY, METHOD_PLAY_ID},{METHOD_TEARDOWN, METHOD_TEARDOWN_ID},{METHOD_NULL, METHOD_NULL_ID}};

static int GET_LOCAL_IP(int socket_fd,char *local_ip)
{
	struct ifreq ifreq;
	struct sockaddr_in *sin;
	if(local_ip == NULL)
	{
		return -1;
	}

	strcpy(ifreq.ifr_name,"eth0");
	if(!(ioctl (socket_fd, SIOCGIFADDR,&ifreq)))
    { 
		sin = (struct sockaddr_in *)&ifreq.ifr_addr;
		sin->sin_family = AF_INET;
       	strcpy(local_ip,inet_ntoa(sin->sin_addr)); 
	}
	return 0;
}

const char *GET_STATUS_CODE(int err)
{
    struct
    {
        const char *token;
        int code;
    } status[] =
    {
        {
            "Continue", 100
        }, {
            "OK", 200
        }, {
            "Created", 201
        }, {
            "Accepted", 202
        }, {
            "Non-Authoritative Information", 203
        }, {
            "No Content", 204
        }, {
            "Reset Content", 205
        }, {
            "Partial Content", 206
        }, {
            "Multiple Choices", 300
        }, {
            "Moved Permanently", 301
        }, {
            "Moved Temporarily", 302
        }, {
            "Bad Request", 400
        }, {
            "Unauthorized", 401
        }, {
            "Payment Required", 402
        }, {
            "Forbidden", 403
        }, {
            "Not Found", 404
        }, {
            "Method Not Allowed", 405
        }, {
            "Not Acceptable", 406
        }, {
            "Proxy Authentication Required", 407
        }, {
            "Request Time-out", 408
        }, {
            "Conflict", 409
        }, {
            "Gone", 410
        }, {
            "Length Required", 411
        }, {
            "Precondition Failed", 412
        }, {
            "Request Entity Too Large", 413
        }, {
            "Request-URI Too Large", 414
        }, {
            "Unsupported Media Type", 415
        }, {
            "Bad Extension", 420
        }, {
            "Invalid Parameter", 450
        }, {
            "Parameter Not Understood", 451
        }, {
            "Conference Not Found", 452
        }, {
            "Not Enough Bandwidth", 453
        }, {
            "Session Not Found", 454
        }, {
            "Method Not Valid In This State", 455
        }, {
            "Header Field Not Valid for Resource", 456
        }, {
            "Invalid Range", 457
        }, {
            "Parameter Is Read-Only", 458
        }, {
            "Unsupported transport", 461
        }, {
            "Internal Server Error", 500
        }, {
            "Not Implemented", 501
        }, {
            "Bad Gateway", 502
        }, {
            "Service Unavailable", 503
        }, {
            "Gateway Time-out", 504
        }, {
            "RTSP Version Not Supported", 505
        }, {
            "Option not supported", 551
        }, {
            "Extended Error:", 911
        }, {
            NULL, -1
        }
    };

    int i;
    for(i = 0; status[i].code != err && status[i].code != -1; ++i);

    return status[i].token;
}

void GET_SDP_INFO(Client_Node *pClient_Node, char *SDP_DESC)
{
/*
	char const* const SdpPrefixFmt =
			"v=0\r\n"	//版本信息
			"o=- %s %s IN IP4 %s\r\n" //<用户名><会话id><版本>//<网络类型><地址类型><地址>
			"c=IN IP4 %s\r\n"			//c=<网络信息><地址信息><连接地址>对ip4为0.0.0.0  here！
			"s=RTSP Session\r\n"		//会话名session id
			"i=N/A\r\n"		//会话信息
			"t=0 0\r\n"		//<开始时间><结束时间>
			"a=recvonly\r\n"
			"m=video %s RTP/AVP 96\r\n\r\n";	//<媒体格式><端口><传送><格式列表,即媒体净荷类型> m=video 5004 RTP/AVP 96
*/

	char SDP_ID[128];	

	time_t t;
    t = time(NULL);
    sprintf(SDP_ID,"%.0f",(float)t+2208988800U);    /*获得NPT时间*/
    printf("sdp id : %s\n",SDP_ID);

	strcpy(SDP_DESC, "v=0\r\n");	
	strcat(SDP_DESC, "o=-");
	strcat(SDP_DESC, SDP_ID);
	strcat(SDP_DESC," ");
	strcat(SDP_DESC, SDP_ID);
	strcat(SDP_DESC," IN IP4 ");
	strcat(SDP_DESC, "224.2.17.12/127");

	strcat(SDP_DESC, "\r\n");
	strcat(SDP_DESC, "s=Unnamed\r\n");
	strcat(SDP_DESC, "i=N/A\r\n");

   	strcat(SDP_DESC, "c=");
   	strcat(SDP_DESC, "IN ");		/* Network type: Internet. */
   	strcat(SDP_DESC, "IP4 ");		/* Address type: IP4. */
	//strcat(SDP_DESC, get_address());
	strcat(SDP_DESC, "224.2.17.12/127");
	strcat(SDP_DESC, "\r\n");
	
   	strcat(SDP_DESC, "t=0 0\r\n");	
	strcat(SDP_DESC, "a=recvonly\r\n");
	/**** media specific ****/
	strcat(SDP_DESC,"m=");
	strcat(SDP_DESC,"video ");
	strcat(SDP_DESC, "1234");
	strcat(SDP_DESC," RTP/AVP ");
	strcat(SDP_DESC,"96\r\n");

	strcat(SDP_DESC,"b=RR:0\r\n");
		/**** Dynamically defined payload ****/
	strcat(SDP_DESC,"a=rtpmap:96");
	strcat(SDP_DESC," ");	
	strcat(SDP_DESC,"H264/90000");
	strcat(SDP_DESC, "\r\n");
	strcat(SDP_DESC,"a=fmtp:96 packetization-mode=1;");
	strcat(SDP_DESC,"profile-level-id=");
	strcat(SDP_DESC,"ASDASDADASD");
	strcat(SDP_DESC,";sprop-parameter-sets=");
	strcat(SDP_DESC,"164848541");
	strcat(SDP_DESC,",");
	strcat(SDP_DESC,"164848511");
	strcat(SDP_DESC,";");
	strcat(SDP_DESC, "\r\n");
	strcat(SDP_DESC,"a=control:trackID=1123");
	strcat(SDP_DESC, "\r\n");
}

int RTSP_REPLY(int status_code, Client_Node *pClient_Node)
{
    char buf[1024];

    sprintf(buf, "%s %d %s\r\nCSeq: %d\r\n", 
	RTSP_VERSION, status_code, GET_STATUS_CODE(status_code), pClient_Node->message.CSeq);

	send(pClient_Node->socket_fd,buf,strlen(buf),0);
    return 0;
}

int RTSP_OPTIONS(Client_Node *pClient_Node)
{
    char buf[1024];

    sprintf(buf, "%s %d %s\r\nCSeq: %d\r\nPublic: OPTIONS,DESCRIBE,SETUP,PLAY,PAUSE,TEARDOWN\r\n", 
	RTSP_VERSION, 200, GET_STATUS_CODE(200), pClient_Node->message.CSeq);

	send(pClient_Node->socket_fd,buf,strlen(buf),0);
    return ERR_NOERROR;
}

int RTSP_DESCRIBE(Client_Node *pClient_Node)
{	
	char buf[4096];
	char SDP_DESC[1024];
    sprintf(buf, "%s %d %s\r\nCSeq: %d\r\n%s: 23 Jan 1997 15:35:06 GMT\r\n%s:\r\n%s:\r\n", 
	RTSP_VERSION, 200, GET_STATUS_CODE(200),pClient_Node->message.CSeq,HDR_DATE,HDR_CONTENTTYPE,HDR_CONTENTLENGTH);

	GET_SDP_INFO(pClient_Node,SDP_DESC);
	strcat(buf, SDP_DESC); 

	send(pClient_Node->socket_fd,buf,strlen(buf),0);

	return ERR_NOERROR;
}

int RTSP_SETUP(Client_Node *pClient_Node)
{	
	char *str;
	char buf[1024];
	int rtp_port  = 0;
	int rtcp_port = 0;

	struct sockaddr_in server_rtp_addr;
	socklen_t addr_len = sizeof(struct sockaddr_in);

	if(strstr(pClient_Node->message.buf, HDR_TRANSPORT) == NULL)
	{
		fprintf(stderr, "can't find Transport ! %s,%i\n", __FILE__, __LINE__);
		RTSP_REPLY(406, pClient_Node);
	}

	if(strstr(pClient_Node->message.buf, RTP_AVP) != NULL || strstr(pClient_Node->message.buf, RTP_AVP_UDP) != NULL)
	{
		str = strstr(pClient_Node->message.buf, "client_port=");

		sscanf(str, "client_port=%d-%d", &rtp_port,&rtcp_port);
		if(rtp_port == 0)
		{
			fprintf(stderr, "can't find rtp_port ! %s,%i\n", __FILE__, __LINE__);
			RTSP_REPLY(406, pClient_Node);
		}
		if(rtcp_port == 0)
		{
			fprintf(stderr, "can't find rtcp_port ! %s,%i\n", __FILE__, __LINE__);
			//RTSP_REPLY(406, pClient_Node);
		}

		if(RTPCreate(pClient_Node) < 0)
		{
			RTSP_REPLY(500, pClient_Node);
			fprintf(stderr, "RTSP server error:RTP create fail!%s,%i\n", __FILE__, __LINE__);
		}
		getsockname(pClient_Node->message.rtp_socket_fd,(struct sockaddr *)&server_rtp_addr,&addr_len);

		memcpy(&pClient_Node->message.rtp_addr,&pClient_Node->client_addr,sizeof(struct sockaddr));
		memcpy(&pClient_Node->message.rtcp_addr,&pClient_Node->client_addr,sizeof(struct sockaddr));
		pClient_Node->message.rtp_addr.sin_port  = htons(rtp_port);
		pClient_Node->message.rtcp_addr.sin_port = htons(rtcp_port);

		sprintf(buf, "%s %d %s\r\nCSeq: %d\r\nDate: 23 Jan 1997 15:35:06 GMT\r\nSession: 47112344\r\nTransport: %s;unicast;\r\nclient_port=%d-%d;server_port=%d-%d\r\n",
		RTSP_VERSION, 200, GET_STATUS_CODE(200), pClient_Node->message.CSeq,RTP_AVP,rtp_port,rtcp_port,htons(server_rtp_addr.sin_port),0);

		send(pClient_Node->socket_fd,buf,strlen(buf),0);

	}else if(strstr(pClient_Node->message.buf, RTP_AVP_TCP) != NULL)
	{
		RTSP_REPLY(406, pClient_Node);
		//no coding
	}else
	{
        fprintf(stderr, "can't find Transport ! %s,%i\n", __FILE__, __LINE__);
		RTSP_REPLY(406, pClient_Node);
	}

	return ERR_NOERROR;
}

int RTSP_PLAY(Client_Node *pClient_Node)
{
	char buf[1024];

	if(strstr(pClient_Node->message.buf, pClient_Node->message.session) == NULL)
	{
		fprintf(stderr, "can't find session ! %s,%i\n", __FILE__, __LINE__);
		RTSP_REPLY(406, pClient_Node);
	}
	
	if(strstr(pClient_Node->message.buf, HDR_RANGE) == NULL)
	{
		fprintf(stderr, "can't find range ! %s,%i\n", __FILE__, __LINE__);
		RTSP_REPLY(406, pClient_Node);	
	}

	sprintf(buf, "%s %d %s\r\nCSeq: %d\r\nDate: 23 Jan 1997 15:35:06 GMT\r\n",
	RTSP_VERSION, 200, GET_STATUS_CODE(200), pClient_Node->message.CSeq);

	send(pClient_Node->socket_fd,buf,strlen(buf),0);
	
	pthread_create(&pClient_Node->message.tidp, NULL, RTPHandle, pClient_Node);

	return ERR_NOERROR;
}

int RTSP_PAUSE(Client_Node *pClient_Node)
{
	return ERR_NOERROR;
}

int RTSP_TEARDOWN(Client_Node *pClient_Node)
{
	char buf[1024];
	sprintf(buf, "%s %d %s\r\nCSeq: %d\r\n",
	RTSP_VERSION, 200, GET_STATUS_CODE(200), pClient_Node->message.CSeq);

	send(pClient_Node->socket_fd,buf,strlen(buf),0);
	RTPDestroy(pClient_Node);
	return ERR_NOERROR;
}

int RTSP_VALIDATE_MESSAGE(Client_Node *pClient_Node)
{
	int i;
	char *str;
    char method[32];
    char url[64];
    char version[32];
	int  CSeq;
	char RTSP_SERVER_IP[32];

	if(sscanf(pClient_Node->message.buf, "%31s %255s %31s\n", method, url, version) != 3) 
	{
		printf("\n%s\n",pClient_Node->message.buf);
		printf("%s\n",method);
		printf("%s\n",url);
		printf("%s\n",version);
		return -1;
	}

//validate method
    for(i = 0; HDR_METHOD[i].method_id != METHOD_NULL_ID; ++i)
	{
		if(strcmp(method,HDR_METHOD[i].method) == 0)
		{
			break;
		}
	}
	if(HDR_METHOD[i].method_id == METHOD_NULL_ID)
	{
		printf("\n validate method error!\n");
		return -1;
	}
	pClient_Node->message.method_id = HDR_METHOD[i].method_id;

//validate url
	if(GET_LOCAL_IP(pClient_Node->socket_fd,RTSP_SERVER_IP) != 0)
	{
		printf("\nRTSP server can't get local IP!\n");
		return -1;
	}
	sprintf(pClient_Node->message.url,RTSP_SERVER_URL,RTSP_SERVER_IP,RTSP_SERVER_PORT,"X");
	printf("RTSP URL:%s\n",pClient_Node->message.url);
	if(strcmp(url,pClient_Node->message.url) != 0)
	{
		printf("\n validate url error!\n");
		//return -1;	
	}

//validate version
	if(strcmp(version,HDR_VERSION) != 0)
	{
		printf("\n validate version error!\n");
		return -1;
	}

//validate CSeq
	str = strstr(pClient_Node->message.buf,HDR_CSEQ);
	if (str == NULL)
	{
		printf("validate CSeq error 0!\n");
		return -1;
	}
	if(sscanf(str,"CSeq:%d",&CSeq) == -1)
	{
		printf("validate CSeq error 1!\n");
		return -1;
	}
	if((pClient_Node->message.CSeq == -1) || (pClient_Node->message.CSeq + 1 == CSeq))
	{
		pClient_Node->message.CSeq = CSeq;
	}
	else
	{
		printf("validate CSeq error 2!\n");
		return -1;
	}

	return 0;
}

int RTSP_STATE_MACHINE(Client_Node *pClient_Node)
{
	if(RTSP_VALIDATE_MESSAGE(pClient_Node) < 0)
	{
		fprintf(stderr,"validate message error!\n");
		return -1;
	}

    switch (pClient_Node->state)
    {
        case INIT_STATE:
        {
			switch (pClient_Node->message.method_id)
            {
				case METHOD_OPTIONS_ID:
					RTSP_OPTIONS(pClient_Node);
                break;

                case METHOD_DESCRIBE_ID:
                    RTSP_DESCRIBE(pClient_Node);
                break;

                case METHOD_SETUP_ID:
					if(RTSP_SETUP(pClient_Node) == ERR_NOERROR)
                    {
                    	pClient_Node->state = READY_STATE;
                        fprintf(stderr,"TRANSFER TO READY STATE!\n");
                    }
                break;

                case METHOD_PLAY_ID:
					RTSP_REPLY(455, pClient_Node);
				break;

                case METHOD_PAUSE_ID:
                    RTSP_REPLY(455, pClient_Node);
                break;
			
                case METHOD_TEARDOWN_ID:
                    RTSP_TEARDOWN(pClient_Node);
                break;

                default:
                    RTSP_REPLY(501, pClient_Node);
				break;
            }
			break;
		}
        
		case READY_STATE:
        {
            switch (pClient_Node->message.method_id)
            {   
				case METHOD_OPTIONS_ID:
                    if (RTSP_OPTIONS(pClient_Node) == ERR_NOERROR)
                    {
						pClient_Node->state = INIT_STATE;
                    }
                break;

                case METHOD_DESCRIBE_ID:
                    RTSP_DESCRIBE(pClient_Node);
                break;
				
				case METHOD_PLAY_ID:                                      //状态迁移为播放态
                   if (RTSP_PLAY(pClient_Node) == ERR_NOERROR)
                    {
                        fprintf(stderr,"\tStart Playing!\n");
                        pClient_Node->state = PLAYING_STATE;
                    }
                break;

                case METHOD_SETUP_ID:
                    if (RTSP_SETUP(pClient_Node) == ERR_NOERROR)    //状态不变
                    {
                        pClient_Node->state = READY_STATE;
                    }
                break;

                case METHOD_TEARDOWN_ID:
                    RTSP_TEARDOWN(pClient_Node);                 //状态变为初始态
                break;

                case METHOD_PAUSE_ID:         			// method not valid this state.
                    RTSP_REPLY(455, pClient_Node);
                break;

                default:
                    RTSP_REPLY(501, pClient_Node);
                break;
            }

            break;
        }

        case PLAYING_STATE:
        {
            switch (pClient_Node->message.method_id)
            {
                case METHOD_PLAY_ID:
                    // Feature not supported
                    fprintf(stderr,"UNSUPPORTED: Play while playing.\n");
                    RTSP_REPLY(501, pClient_Node);       // Option not supported
                break;
/*				//不支持暂停命令
                case RTSP_ID_PAUSE:              	//状态变为就绪态
                    if (RTSP_pause(pClient_Node) == ERR_NOERROR)
                    {
                    	pRtspSess->cur_state = READY_STATE;
                    }
                    break;
*/
                case METHOD_TEARDOWN_ID:
                    RTSP_TEARDOWN(pClient_Node);        //状态迁移为初始态
					pClient_Node->state = INIT_STATE;
					return -1;
                break;

                case METHOD_OPTIONS_ID:
                break;

                case METHOD_DESCRIBE_ID:
                    RTSP_DESCRIBE(pClient_Node);
                break;

                case METHOD_SETUP_ID:
                break;
            }

            break;
        }

        default:
        {
			break;
		}
    }

	return 0;
}
