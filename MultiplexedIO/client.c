/*** socket/demo2/client.c ***/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(){
    int    server_fd;
    struct sockaddr_in server_addr;
    socklen_t addr_size = sizeof(server_addr);

    /* STEP 1:
    Create a socket to talk to the server;
    */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(25555);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* STEP 2:
    Try to connect to the server.
    */
    connect(server_fd,
            (struct sockaddr *) &server_addr,
            addr_size);

    char buffer[1024];

    while(1) {

        /* Read a line from terminal
           and send that line to the server
         */
        printf("[Client]: "); fflush(stdout);
        scanf("%s", buffer);
        send(server_fd, buffer, strlen(buffer), 0);

        /* Receive response from the server */
        int recvbytes = recv(server_fd, buffer, 1024, 0);
        if (recvbytes == 0) break;
        else {
            buffer[recvbytes] = 0;
            printf("[Server]: %s", buffer); fflush(stdout);
        }
    }

    close(server_fd);

  return 0;
}
