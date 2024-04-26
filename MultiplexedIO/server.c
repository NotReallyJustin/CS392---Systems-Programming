/*** socket/demo2/server.c ***/

#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

int main()
{
    int server_fd;
    int client_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in in_addr;
    socklen_t addr_size = sizeof(in_addr);

    /* STEP 1
        Create and set up a socket
    */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(25555);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* STEP 2
        Bind the file descriptor with address structure
        so that clients can find the address
    */
    bind(server_fd,
         (struct sockaddr *)&server_addr,
         sizeof(server_addr));

    /* STEP 3
        Listen to at most 5 incoming connections
    */
    if (listen(server_fd, 5) == 0)
        printf("Listening\n");
    else
        perror("listen");

    /* STEP 4
        Accept connections from clients
        to enable communication

        This is where multiplexed I/O comes in
    */

    // Create a file descriptor set
    fd_set myset;
    FD_SET(server_fd, &myset);          // Add server_fd to the file descriptor set myset
    int max_fd = server_fd;         // The maximum file descriptor number
    int MAX_CONN = 2; // Maximum # of connections
    int n_conn = 0; // Number of connections
    int cfds[MAX_CONN];     // Client file descriptors

    // These are just here because we want to read files later
    int recvbytes = 0;          // How mnay bytes we recieved
    char buffer[1024];          // Read buffer
    char *receipt = "Read\n";   

    // Populate file descriptors with -1 to show that they're invalid
    // We can't leave it at 0 (which C defaults to) because ... well... 0 is a file descriptor
    for (int i = 0; i < MAX_CONN; i++)
    {
        cfds[i] = -1;
    }

    while (1)
    {
        /*
            Note well: Upon return, each of the file descriptor sets is
            modified in place to indicate which file descriptors are
            currently "ready".  Thus, if using select() within a loop, the
            sets must be reinitialized before each call.

            Basically, that means when select detects a readied file I/O, the rest of the file descriptors
            that it is modified gets kicked out. We need to add them back in.

            That's what the for loop does
        */
        FD_SET(server_fd, &myset);
        for (int i = 0; i < MAX_CONN; i++)
        {
            if (cfds[i] != -1)
            {
                printf("%d\n", i);

                // Set other file descriptors into myset
                FD_SET(cfds[i], &myset);

                if (cfds[i] > max_fd)
                {
                    // Change the maximum file descriptor number
                    max_fd = cfds[i];
                }
            }
        }

        // We used multiplexed I/O to manage all these sockets
        // Essentially, select() checks and reports when one of them is ready to read stuff
        select(max_fd + 1, &myset, NULL, NULL, NULL);

        // Tests if there's something to read from the server_fd
        if (FD_ISSET(server_fd, &myset))
        {
            client_fd = accept(server_fd,
                               (struct sockaddr *)&in_addr,
                               &addr_size);

            if (n_conn < MAX_CONN)
            {
                cfds[n_conn] = client_fd;
                n_conn++;
                puts("New connection detected!");
            }
            else
            {
                puts("Max connections reached!");
                close(client_fd);
            }
        }

        // Now, let's start reading from the sockets
        // Loop through all of them
        for (int i = 0; i < MAX_CONN; i++)
        {
            // Tests if there is something to read from the client sockets (files, bc sockets == files in C)
            // Of course, if the file descriptors are not set. don't try to read that
            if (cfds[i] != -1 && FD_ISSET(cfds[i], &myset))
            {
                // Read the socket
                recvbytes = read(cfds[i], buffer, 1024);

                if (recvbytes > 0)
                {
                    buffer[recvbytes] = '\0';           // Put a null terminator at the end so we can print this out
                    printf("[Client] %s\n", buffer);
                    write(cfds[i], receipt, strlen(receipt));           // Send a read reciept to the client so they know stuff passed
                }
                else if (recvbytes == 0)            // EOF - Client Disconnected
                {
                    n_conn--;                       // We lost 1 connection
                    close(cfds[i]);                 // Close the file descriptor
                    cfds[i] = -1;                   // Reset the file descriptor
                    printf("Connection lost\n");
                }
                else                                // If it returns a negative number we are erroring
                {
                    fprintf(stderr, "Error: %s\n", strerror(errno));
                }
            }
        }
    }

    // Close tge server file descriptor. The client will close on itself.
    close(server_fd);

    return 0;
}
