// udp_server.cpp
// Borland compilation
// bcc32 udp_server.cpp ws2_32.lib

#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h> /* memset() */
#include "winsock2.h"
#include <iostream>

#define DEFAULT_PORT 1500
#define MAX_MSG 100

int do_server(const char *name, int port)
{
    SOCKET				sd;
    int					clientLength;
    struct sockaddr_in	clientAddress,
    serverAddress;
    char				msg[MAX_MSG + 1];	/* + 1 for terminating \0 */
    
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd == INVALID_SOCKET) {
        printf("%s: cannot open socket \n", name);
        return 1;
    }
    
    /* bind local server port */
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(port);
    int rc = bind(sd, (struct sockaddr *) &serverAddress, sizeof(serverAddress));
    if (rc < 0) {
        printf("%s: cannot bind port number %d \n", name, port);
        return 1;
    }
    
    printf("%s: waiting for data on port UDP %d\n", name, port);
    
    int return_code = 0;
    while (true) {
        /* receive message */
        clientLength = sizeof(clientAddress);
        int n = recvfrom(sd, msg, sizeof(msg) - 1, 0,
                         (struct sockaddr *)&clientAddress, &clientLength);
        
        if (n > 0) {
            msg[n] = '\0';	/* add terminating \0 */
            printf("%s %d %s\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port), msg);
            
            /* send it back to the client */
            int slen = sendto(sd, msg, n, 0,
                              (struct sockaddr *)&clientAddress, clientLength);
            if (slen <0) {
                printf("%n: sendto failed\n", name);
                return_code = 4;
                break;
            } else {
                //printf("sent back %d bytes\n", slen);
            }
        } else {
            printf("recvfrom returned %d\n", n);
        }
    }
    return return_code;
}

int main(int argc, char *argv[]) {
    WSAData wsaData;
    int nCode;
    int server_port = DEFAULT_PORT;
    
    if ((nCode = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
        std::cerr<<"WSAStartup() returned error code "<<nCode<<"."<<std::endl;
        return 255;
    }
    
    const char SEP = '\\';
    char *progName = argv[0];
    // skip over the path separators
    for (char *p = argv[0]; *p != '\0'; p++) {
        if (*p == SEP) progName = p + 1;
    }
    if (argc >= 2) {
        sscanf(argv[1], "%i", &server_port);
        
    }
    int rc = do_server(progName, server_port);
    
    WSACleanup();
    return rc;
}
