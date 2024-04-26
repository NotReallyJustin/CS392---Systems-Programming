/*
    Justin Chen
    I pledge my honor that I have abided by the Stevens Honor System
*/

// Importing libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

/**
 * Create a IPV4 TCP connection to a server socket.
 * @param ip_addr IP address to connect to
 * @param port Port to connect to
 * @returns The file descriptor of the server socket
 * @throw If the connection process fails, perror() will be thrown and the program will terminate.
*/
int create_connection(char* ip_addr, int port)
{
    // Create a connection to talk to a server
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    // 🚨 Error checking: if socket creation fails
    if (server_fd == -1)
    {
        perror("Error in socket creation");
        exit(EXIT_FAILURE);
    }

    // Connect to a server address. But first, create the server_address struct.
    struct sockaddr_in server_address;
    socklen_t addr_size = sizeof(server_address);
    memset(&server_address, 0, sizeof(addr_size));         // Clear the struct to 0 out memory

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(ip_addr);

    // Now we can connect.
    // 🚨 Error checking: if server connection failed
    if (connect(server_fd, (struct sockaddr *)&server_address, addr_size) == -1)
    {
        perror("Error in server connection:");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

/**
 * Parse the command line arguments.
 * Then, it will connect the client to a server.
 * @param server_fd {Outbound} Pointer to int to store the server socket fd in
*/
void parse_connect(int argc, char** argv, int* server_fd)
{
    // ⭐ Parse command line arguments using getopt()
    int opt;

    // We'll set up the default client parameters
    char ip_addr[16] = "127.0.0.1";
    int port = 25555;

    // Loop through all possible opt-strings
    opterr = 0; // Prevent getopt() from printing its own error message.
    while ((opt = getopt(argc, argv, "i:p:h")) != -1)
    {
        switch(opt)
        {           
            // IP addr
            case 'i':
                strcpy(ip_addr, optarg);
                ip_addr[strlen(optarg)] = '\0';
            break;

            // Port
            case 'p':
                char* port_str = strdup(optarg);
                port = atoi(port_str);

                // 🚨 Error checking: The port is not a number
                if (port == 0)
                {
                    fprintf(stderr, "Error: Invalid port number %s.\n", port_str);
                    exit(EXIT_FAILURE);
                }

                // strdup forces a malloc.
                free(port_str);
            break;
            
            // Display help message
            case 'h':
                printf("Usage: %s [-i IP_address] [-p port_number] [-h]\n\n", argv[0]);
                puts("  -i IP_address           Default to \"127.0.0.1\";");
                puts("  -p port_number          Default to 25555;");
                puts("  -h                      Display this help info.");
                exit(EXIT_SUCCESS);
            break;

            // Unknown case - optopt will contain the option name
            case '?':
                fprintf(stderr, "Error: Unknown option '-%c' recieved.\n", optopt);
                exit(EXIT_FAILURE);
            break;
        }
    }

    *server_fd = create_connection(ip_addr, port);

    // Do an extra listen upon connection

    // Create a temporary buffer to store server messages.
    char buffer[1024];

    int recvbytes = recv(*server_fd, buffer, sizeof(buffer), 0);
    if (recvbytes == 0)
    {
        // If we get an EOF signal, terminate client.
        char* reject = "Sorry, the game is at max capacity.";
        puts(reject);
        exit(EXIT_SUCCESS);
    }
    else 
    {
        // Otherwise, send the server output.
        buffer[recvbytes] = '\0';
        puts(buffer);
    }
}

int main(int argc, char** argv)
{
    // Start the server connection.
    int server_fd;
    parse_connect(argc, argv, &server_fd);

    // Create a buffer to store server messages.
    char buffer[1024];

    // Now, we need multiplexed I/O for the client as well, especially since both fgets from stdin and recv are both blocking.
    // Because C is dumb, we'll need to implement select on both sockets
    fd_set monitoring_set;

    // 💥 Enter the main loop - listen from and send messages to server while handling stdin
    while (1) 
    {
        // 💥 Monitor both stdin and server file descriptor for readable input
        // First, chuck both of them in a set
        FD_SET(server_fd, &monitoring_set);
        FD_SET(STDIN_FILENO, &monitoring_set);

        // Use select to monitor all the sockets for readable fds
        // The max_fd should just be your server_fd, since we didn't open any more "files"
        select(server_fd + 1, &monitoring_set, NULL, NULL, NULL);

        // 💥 Server-Side code: Monitor things coming in from the server
        if (FD_ISSET(server_fd, &monitoring_set))
        {
            // Now, wait to recieve response from the server
            int recvbytes = recv(server_fd, buffer, sizeof(buffer), 0);

            if (recvbytes == 0)
            {
                // If we recieve an EOF signal, break.
                puts("Connection Ended.");
                break;
            }
            else
            {
                // Otherwise, print the server output
                buffer[recvbytes] = '\0';
                puts(buffer);
            }
        }
        
        // 💥 Client-Side Code: Read from stdin and send output
        if (FD_ISSET(STDIN_FILENO, &monitoring_set))
        {
            // Read a line from the terminal (if possible) and send it to the server
            fgets(buffer, sizeof(buffer), stdin);

            // 🚨 Error Checking: Message sending failed
            if (send(server_fd, buffer, strlen(buffer), 0) == -1)
            {
                perror("Failed to send message");
                continue;
            }
        }
    }

    // At the end, close the server fd.
    close(server_fd);

    return EXIT_SUCCESS;
}