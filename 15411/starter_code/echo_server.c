/******************************************************************************
* echo_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
* Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
*          Wolf Richter <wolf@cs.cmu.edu>                                     *
*                                                                             *
*******************************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include<sys/types.h>
#include<sys/select.h>
#include<unistd.h>
#include<arpa/inet.h>

#define ECHO_PORT 9999
#define BUF_SIZE 4096

int close_socket(int sock)
{
    if (close(sock))
    {
        fprintf(stderr, "Failed closing socket.\n");
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    
    fd_set master;
    fd_set read_fds;
    int fd_max; 

    int sock, client_sock;
    ssize_t readret;
    socklen_t cli_size;
    struct sockaddr_in addr, cli_addr;
    char remoteIp[INET_ADDRSTRLEN];
    char buf[BUF_SIZE];

    int i;

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    fprintf(stdout, "----- Echo Server -----\n");
    
    /* all networked programs must create a socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Failed creating socket.\n");
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(ECHO_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    /* servers bind sockets to ports---notify the OS they accept connections */
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)))
    {
        close_socket(sock);
        fprintf(stderr, "Failed binding socket.\n");
        return EXIT_FAILURE;
    }


    if (listen(sock, 5))
    {
        close_socket(sock);
        fprintf(stderr, "Error listening on socket.\n");
        return EXIT_FAILURE;
    }

    fd_max = sock;
    FD_SET(sock, &master);

    /* finally, loop waiting for input and then write it back */
    while (1)
    {
        read_fds = master;
        if(select(fd_max+1, &read_fds, NULL, NULL, NULL) == -1){
            fprintf(stderr, "Error select.\n");
            return EXIT_FAILURE;
        }

        for(i = 0; i <= fd_max; i++){
            if(FD_ISSET(i, &read_fds)){
                if(i == sock){
                    cli_size = sizeof(cli_addr);
                    if((client_sock = accept(sock, (struct sockaddr*) &cli_addr, &cli_size)) == -1){
                        fprintf(stderr,"accept error.\n");
                        return EXIT_FAILURE;
                    }else{
                        FD_SET(client_sock, &master);
                        if(client_sock > fd_max){
                            fd_max = client_sock;
                        }

                        printf("echoServer: a new connection from %s on sock %d.\n ", 
                                    inet_ntop(cli_addr.sin_family,&cli_addr.sin_addr,remoteIp,
                                        INET_ADDRSTRLEN),
                                    client_sock
                        );
                        
                    }
                }else{
                    printf("recive data on sock %d.\n", i);
                    readret = 0;
                    if((readret = recv(i, buf, BUF_SIZE, 0)) >= 1){
                        printf("recieve data %zd.\n",readret);
                        if(send(i, buf, readret, 0) != readret){
                            close(i);
                            fprintf(stderr, "Error send to client.\n");
                            
                        }
                        memset(buf, 0, BUF_SIZE);
                    }else if(readret <= 0){
                    
                        FD_CLR(i, &master);  
                        close(i);
                    }
                    
                    
                }
            }
        }

        
    }   

    close_socket(sock);

    return EXIT_SUCCESS;
}

