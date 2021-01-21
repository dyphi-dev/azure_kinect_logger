//
// Simple listener.c program for UDP multicast
//
// Adapted from:
// http://ntrg.cs.tcd.ie/undergrad/4ba2/multicast/antony/example.html
//
// Changes:
// * Compiles for Windows as well as Linux
// * Takes the port and group on the command line
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32")

#include "UDP_listener.h"
#include "recorder.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <thread>
#include <chrono>

char group[20];
int port;
WSADATA wsaData;
int fd;
struct sockaddr_in addr;
struct ip_mreq mreq;
int addrlen;
unsigned char nbytes;
char msgbuf[MSGBUFSIZE];

char recording_start_gait_flag = false;
char recording_start_s2s_flag = false;
char recording_start_bal_flag = false;

int UDP_Listener_Init(void);
void UDP_msg_handler(void);

int UDP_Listener_Init(void)
{

    //group[20] = "239.255.255.250";
    sprintf_s(group, "%s", "239.255.255.250");
    port = 23232;


#ifdef _WIN32
    //
    // Initialize Windows Socket API with given VERSION.
    //
    if (WSAStartup(0x0101, &wsaData)) {
        perror("WSAStartup");
        return 1;
    }
#endif

    // create what looks like an ordinary UDP socket
    //
    fd = (int)socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    // allow multiple sockets to use the same PORT number
    //
    u_int yes = 1;
    if (
        setsockopt(
            fd, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes)
        ) < 0
        ) {
        perror("Reusing ADDR failed");
        return 1;
    }

    // set up destination address
    //

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // differs from sender
    addr.sin_port = htons(port);

    // bind to receive address
    //
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    // use setsockopt() to request that the kernel join a multicast group
    //

    mreq.imr_multiaddr.s_addr = inet_addr(group);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0) {
        perror("setsockopt");
        return 1;
    }
    return 0;
}


void UDP_msg_handler(void)
{
    //addrlen = sizeof(addr);
    //nbytes = recvfrom(fd, msgbuf, MSGBUFSIZE, 0, (struct sockaddr*)&addr, &addrlen);
    //if (nbytes < 0) {
    //    perror("recvfrom");
    //    return;
    //}
    //
    //msgbuf[nbytes] = '\0';
    //std::cout << "msg received : " << msgbuf << std::endl;
    for (;;)
    {

        addrlen = sizeof(addr);
        nbytes = recvfrom(fd, msgbuf, MSGBUFSIZE, 0, (struct sockaddr*)&addr, &addrlen);
        if (nbytes < 0) {
            perror("recvfrom");
            return;
        }
        msgbuf[nbytes] = '\0';
        std::cout << "msg received : " << msgbuf << std::endl;

        if (strcmp(msgbuf, "gait_start") == 0)
        {
            recording_start_gait_flag = true;
        }
        else if (strcmp(msgbuf, "gait_finish") == 0)
        {
            exiting = true;
            recording_start_gait_flag = false;
        }

        if (strcmp(msgbuf, "s2s_start") == 0)
        {
            recording_start_s2s_flag = true;
        }
        else if (strcmp(msgbuf, "s2s_finish") == 0)
        {
            exiting = true;
            recording_start_s2s_flag = false;
        }

        if (strcmp(msgbuf, "bal_start") == 0)
        {
            recording_start_bal_flag = true;
        }
        else if (strcmp(msgbuf, "bal_finish") == 0)
        {
            exiting = true;
            recording_start_bal_flag = false;
        }

        //std::this_thread::sleep_for(std::chrono::milliseconds(200));
        //std::cout << "msg handler" << msgbuf << std::endl;
    }

}

