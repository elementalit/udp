// udp_client.cpp
// Borland compilation
// bcc32 udp_client.cpp ws2_32.lib

#include <stdlib.h> /* for exit() */
#include <sys/types.h>
#include <stdio.h>
#include <string.h> /* memset() */
#include "winsock2.h"
#include <iostream>

#define DEFAULT_PORT 1500
#define MAX_MSG 100

int do_client(const char *name, char *host_name, int port)
{
    SOCKET    sd;
    int        rc,
    flags = 0;
    struct sockaddr_in cliAddr, remoteServAddr;
    
    hostent			*remoteHost;
    unsigned int	addr;
    
    // If host_name is a name (starts with a letter), use gethostbyname()
    // If not, use gethostbyaddr()
    if (isalpha(host_name[0])) {   // host address is a name
        // if hostname terminated with newline '\n', remove and zero-terminate
        if (host_name[strlen(host_name)-1] == '\n')
        host_name[strlen(host_name)-1] = '\0';
        remoteHost = gethostbyname(host_name);
    }
    else  {
        addr = inet_addr(host_name);
        remoteHost = gethostbyaddr((char *)&addr, 4, AF_INET);
    }
    
    int myerror = WSAGetLastError();
    if (myerror != 0) {
        if (myerror == 11001) {
            printf("error = %d host not found...\nExiting.\n", myerror);
        } else {
            printf("error = %d\nExiting.\n", myerror);
        }
        return 1;
    }
    
    printf("%s: sending data to '%s' (IP : %s) \n", name, remoteHost->h_name,
           inet_ntoa(*(struct in_addr *)remoteHost->h_addr_list[0]));
    
    remoteServAddr.sin_family = remoteHost->h_addrtype;
    memcpy((char *)&remoteServAddr.sin_addr.s_addr,
           remoteHost->h_addr_list[0],
           remoteHost->h_length);
    remoteServAddr.sin_port = htons(port);
    
    /* socket creation */
    sd = socket(AF_INET,SOCK_DGRAM,0);
    if (sd == INVALID_SOCKET) {
        printf("%s: cannot open socket \n",name);
        return 1;
    }
    
    /* bind any port */
    cliAddr.sin_family = AF_INET;
    cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    cliAddr.sin_port = htons(0);
    
    rc = bind(sd, (struct sockaddr *)&cliAddr, sizeof(cliAddr));
    if (rc<0) {
        printf("%s: cannot bind port\n", name);
        return 1;
    }
    
    /* send data */
    const	int bufSize = 1000;
    char	buffer[bufSize];
    puts("Enter message (Ctrl-Z to exit): ");
    while (fgets(buffer, bufSize, stdin))	// returns NULL when at end of file
    {
        // remove newline at the end of the string
        int len = strlen(buffer);
        if (buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }
        if (len == 0) continue;	// ignore blank line
        
        //printf("sending %s\n", buffer);
        rc = sendto(sd, buffer, (int)strlen(buffer), flags,
                    (sockaddr *)&remoteServAddr, (int)sizeof(remoteServAddr));
        if (rc<0) {
            printf("%s: cannot send data\n", name);
            closesocket(sd);
            return 1;
        } else {
            struct  sockaddr_in from;
            int     fromlen;
            
            //printf("sent %d bytes\n", rc);
            //Sleep(1000);
            rc = recvfrom(sd, buffer, bufSize, flags, (sockaddr *)&from, &fromlen);
            buffer[rc] = '\0';
            printf("%s %d %s\n", inet_ntoa(from.sin_addr), ntohs(from.sin_port), buffer);
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    const char SEP = '\\';
    char *progName = argv[0];
    // skip over the path separators
    for (char *p = argv[0]; *p != '\0'; p++) {
        if (*p == SEP) progName = p + 1;
    }
    
    /* check command line args */
    if (argc < 3) {
        printf("usage : %s server port\n", progName);
        return 0;
    }
    
    WSAData wsaData;
    int nCode;
    if ((nCode = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
        std::cerr<<"WSAStartup() returned error code "<<nCode<<"."<<std::endl;
        return 255;
    }
    
    int port = atoi(argv[2]);
    if (port == 0) port = DEFAULT_PORT;
    
    int rc = do_client(progName, argv[1], port);
    
    //system("pause");
    WSACleanup();
    return rc;
}
