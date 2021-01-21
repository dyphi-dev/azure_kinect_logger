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

#include "tcp_server.h"
#include "recorder.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <thread>
#include <chrono>


WSADATA     wsaData;
SOCKET      hServSock, hClntSock;
SOCKADDR_IN servAddr, clntAddr;

short   port = 23232;
int     szClntAddr;
char    message[100];
char    log_message[1000];
FILE* fp_log;

char recording_start_gait_flag = false;
char recording_start_s2s_flag = false;
char recording_start_bal_flag = false;

void ErrorHandling(char* message);
void EventLogging(char* message);

void TCP_Msg_Handler(void)
{
    char log_time[100];
    struct tm pLocal;
    time_t curTime;


    for (;;)
    {

        // initialize winsock
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            ErrorHandling((char*)"WSAStartup() error!");

        // create TCP socket
        hServSock = socket(PF_INET, SOCK_STREAM, 0);

        //struct timeval tv;
        //tv.tv_sec = 5000;  /* 5 Secs Timeout */
        //setsockopt(hServSock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(struct timeval));


        if (hServSock == INVALID_SOCKET)
            ErrorHandling((char*)"socket() error!");

        // address information of socket
        memset(&servAddr, 0, sizeof(servAddr));

        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = htonl(INADDR_ANY);  // use current PC's ip address
        servAddr.sin_port = htons(port);        // port number

        // bind address
        if (bind(hServSock, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
            ErrorHandling((char*)"bind() error!");


        if (listen(hServSock, 5) == SOCKET_ERROR)    // waiting for connection
            ErrorHandling((char*)"listen() error!");

        // accept the connection
        szClntAddr = sizeof(clntAddr);
        hClntSock = accept(hServSock, (SOCKADDR*)&clntAddr, &szClntAddr);   // communication with client

        if (hClntSock == INVALID_SOCKET)
            ErrorHandling((char*)"accept() error!");


        //logging file generation
        curTime = time(NULL);
        localtime_s(&pLocal, &curTime);
        sprintf_s(log_time, "event_log_%04d_%02d_%02d_%02d_%02d_%02d.txt", pLocal.tm_year + 1900, pLocal.tm_mon + 1, pLocal.tm_mday, pLocal.tm_hour, pLocal.tm_min, pLocal.tm_sec);
        fopen_s(&fp_log, log_time, "w");

        curTime = time(NULL);
        localtime_s(&pLocal, &curTime);
        sprintf_s(log_message,"[%04d_%02d_%02d_%02d:%02d:%02d] client connected from %d.%d.%d.%d\n", pLocal.tm_year + 1900, pLocal.tm_mon + 1, pLocal.tm_mday, pLocal.tm_hour, pLocal.tm_min, pLocal.tm_sec,
                                                                                        clntAddr.sin_addr.S_un.S_un_b.s_b1,
                                                                                        clntAddr.sin_addr.S_un.S_un_b.s_b2,
                                                                                        clntAddr.sin_addr.S_un.S_un_b.s_b3,
                                                                                        clntAddr.sin_addr.S_un.S_un_b.s_b4);
        EventLogging(log_message);

        int msg_timeout_counter = 0;
        for (;;)
        {
            int strLen = recv(hClntSock, message, sizeof(message) - 1, 0);

            //if (strLen == -1)
            //    ErrorHandling((char*)"read() error!");

            curTime = time(NULL);
            localtime_s(&pLocal, &curTime);
            sprintf_s(log_message, "[%04d_%02d_%02d_%02d:%02d:%02d] message from client: %s \n", pLocal.tm_year + 1900, pLocal.tm_mon + 1, pLocal.tm_mday, pLocal.tm_hour, pLocal.tm_min, pLocal.tm_sec, message);
            EventLogging(log_message);



            if (strLen == -1)
            {
                msg_timeout_counter++;
                //printf("msg timeout \n");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (msg_timeout_counter > 10) break;
            }
            else
            {
                message[strLen] = 0;

                if (strcmp(message, "bal_start") == 0)
                {
                    exiting = false;
                    recording_start_bal_flag = true;
                }
                else if (strcmp(message, "bal_finish") == 0)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                    exiting = true;
                    recording_start_bal_flag = false;
                }

                if (strcmp(message, "gait_start") == 0)
                {
                    exiting = false;
                    recording_start_gait_flag = true;
                }
                else if (strcmp(message, "gait_finish") == 0)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                    exiting = true;
                    recording_start_gait_flag = false;
                }

                if (strcmp(message, "s2s_start") == 0)
                {
                    exiting = false;
                    recording_start_s2s_flag = true;
                }
                else if (strcmp(message, "s2s_finish") == 0)
                {
                    exiting = true;
                    recording_start_s2s_flag = false;
                }

                if (strcmp(message, "test_cancel") == 0)
                {
                    exiting = true;
                    recording_start_bal_flag = false;
                    recording_start_gait_flag = false;
                    recording_start_s2s_flag = false;
                }


                for (int i = 0; i < 100; i++) message[i] = 0;

                msg_timeout_counter = 0;
                //const auto p1 = std::chrono::system_clock::now();
                //timestamp_last_message_received = std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();

            }
        }


        closesocket(hClntSock);
        closesocket(hServSock);
        WSACleanup();

        curTime = time(NULL);
        localtime_s(&pLocal, &curTime);
        sprintf_s(log_message,"[%04d_%02d_%02d_%02d:%02d:%02d] disconnected\n", pLocal.tm_year + 1900, pLocal.tm_mon + 1, pLocal.tm_mday, pLocal.tm_hour, pLocal.tm_min, pLocal.tm_sec);
        EventLogging(log_message);

        fclose(fp_log);
            
        //addrlen = sizeof(addr);
        //nbytes = recvfrom(fd, msgbuf, MSGBUFSIZE, 0, (struct sockaddr*)&addr, &addrlen);
        //if (nbytes < 0) {
        //    perror("recvfrom");
        //    return;
        //}
        //msgbuf[nbytes] = '\0';
        //std::cout << "msg received : " << msgbuf << std::endl;
        //
        //if (strcmp(msgbuf, "gait_start") == 0)
        //{
        //    exiting = false;
        //    recording_start_gait_flag = true;
        //}
        //else if (strcmp(msgbuf, "gait_finish") == 0)
        //{
        //    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        //    exiting = true;
        //    recording_start_gait_flag = false;
        //}
        //
        //if (strcmp(msgbuf, "s2s_start") == 0)
        //{
        //    exiting = false;
        //    recording_start_s2s_flag = true;
        //}
        //else if (strcmp(msgbuf, "s2s_finish") == 0)
        //{
        //    exiting = true;
        //    recording_start_s2s_flag = false;
        //}
        //
        //if (strcmp(msgbuf, "bal_start") == 0)
        //{
        //    exiting = false;
        //    recording_start_bal_flag = true;
        //}
        //else if (strcmp(msgbuf, "bal_finish") == 0)
        //{
        //    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        //    exiting = true;
        //    recording_start_bal_flag = false;
        //}

        //std::this_thread::sleep_for(std::chrono::milliseconds(200));
        //std::cout << "msg handler" << msgbuf << std::endl;
    }

}

void ErrorHandling(char* _message)
{
    fputs(_message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void EventLogging(char* _message)
{
    printf("%s",_message);
    fprintf(fp_log, "%s", _message);
}