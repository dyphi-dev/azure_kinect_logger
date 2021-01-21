#ifndef UDP_LISTENER_H
#define UDP_LISTENER_H

#ifdef _WIN32
#include <Winsock2.h> // before Windows.h, else Winsock 1 conflict
#include <Ws2tcpip.h> // needed for ip_mreq definition for multicast
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#endif


#define MSGBUFSIZE 256

extern char group[20];
extern int port;
extern WSADATA wsaData;
extern int fd;
extern struct sockaddr_in addr;
extern struct ip_mreq mreq;
extern int addrlen;
extern unsigned char nbytes;
extern char msgbuf[MSGBUFSIZE];

extern char recording_start_bal_flag;
extern char recording_start_gait_flag;
extern char recording_start_s2s_flag;


extern int UDP_Listener_Init(void);
extern void UDP_msg_handler(void);


#endif /* UDP_LISTENER_H */