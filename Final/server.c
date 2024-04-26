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
#include <sys/socket.h>
#include <sys/select.h> 
#include <netinet/in.h>
#include <errno.h>

// Predefined macros to interpret our lines read from the question file based on the (i-1)th line they appear on
#define QUESTION 0
#define CHOICES 1
#define RIGHT_ANSWER 2
#define LINE_SEPERATOR 3

/**
 * The structure for a question entry.
*/
struct Entry 
{
    /**
     * Question Prompt
    */
    char prompt[1024];

    /**
     * Question Answers
    */
    char options[3][50];

    /**
     * Index of the correct answer
    */
    int answer_idx;
};

/**
 * The structure to track a player's points
*/
struct Player 
{
    /**
     * File descriptor to client socket
    */
    int fd;

    /**
     * Player score
    */
    int score;

    /**
     * Player name
    */
    char name[128];
};

/**
 * Given a string, replace the source character with the destination character
 * Prerequisite: The memory of str can be modified
 * @param str The string to run replace on
 * @param source The character to replace
 * @param dest The character to replace the source with 
*/
void str_replace(char* str, char source, char dest)
{
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] == source)
        {
            str[i] = dest;
        }
    }
}

/**
 * Create a IPV4 TCP server that listens on a specified ip address and port.
 * @param ip_addr The IP address for the TCP server
 * @param port The port to bind to (and listen on)
 * @param MAX_CONNECTIONS The maximum # of connections the server can support at a time
 * @returns The file descriptor of the server socket
 * @throw If the server creation process fails, perror() will be thrown and the program will terminate.
*/
int create_server(char* ip_addr, int port, int MAX_CONNECTIONS)
{
    // Create a socket. This returns the file descriptor.
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 🚨 Error checking: if socket creation fails
    if (server_fd == -1)
    {
        perror("Error in socket creation");
        exit(EXIT_FAILURE);
    }

    // Bind to an IP address. First, we will format it using the struct.
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));         // Clear the struct to 0 out memory
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(ip_addr);

    // 🚨 Error checking: if socket binding fails
    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        perror("Error in server binding");
        exit(EXIT_FAILURE);
    }

    // Listen on the port. If it's successful, print a confirmation message.
    if (listen(server_fd, MAX_CONNECTIONS) == 0)
    {
        puts("Welcome to 392 Trivia!");
    }
    else
    {
        // 🚨 Error checking: server listening does not work
        perror("Error in server listening");
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

/**
 * Read the questions file, and appends the Question Entries inside $arr.
 * @param arr The array to append the Questions Entries into. This can be hardcoded as 50.
 * @param filename The name of the file to read question.txt from
 * @returns The number of actual questions read
*/
int read_questions(struct Entry* arr, char* filename)
{
    int num_questions = 0;

    // Get ready to read lines
    char* line;
    size_t num_allocated = 0;   // The number of bytes allocated in line. We will not use this though
    ssize_t bytes_read = 0;

    FILE* question_file = fopen(filename, "r");

    // 🚨 Error checking: If there was a problem opening the question file
    if (question_file == NULL)
    {
        fprintf(stderr, "Error reading question file %s: %s.", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Loop through the question file while populating the entry array.
    int question_position = 0;      // We will compare this with macros to see what to do

    // Create the current entry we're filling in. Zero it out though.
    struct Entry curr_entry;
    memset(&curr_entry, 0, sizeof(curr_entry));

    while ((bytes_read = getline(&line, &num_allocated, question_file)) != -1)
    {
        // Get rid of the \n at the end since strtok is really bad at parsing those
        line[strlen(line) - 1] = '\0';

        if (question_position == QUESTION)
        {
            strncpy(curr_entry.prompt, line, sizeof(curr_entry.prompt));
        }
        else if (question_position == CHOICES)
        {
            // Split the choices by delimiter
            char* token = strtok(line, " ");

            // We can hard code the # of options since we are explicitly given a very restrictive struct
            for (int i = 0; i < 3; i++)
            {
                // Duplicating token because I do not for the life of god trust C to handle this properly
                char* token_dup = strdup(token);
                str_replace(token_dup, '_', ' ');

                strncpy(curr_entry.options[i], token_dup, sizeof(curr_entry.options[i]));
                
                // Free the token_dup because strdup implicitly malloc()'s
                free(token_dup);
                token = strtok(NULL, " ");
            }
        }
        else if (question_position == RIGHT_ANSWER)
        {
            // Before we start, replace _ with ' '
            str_replace(line, '_', ' ');

            int answer_idx = -1;

            // Do a linear search to find the answer option that matches the right answer
            // We hardcode the length of options since that was given to us in the specs
            for (int i = 0; i < 3; i++)
            {
                // If the current option matches the correct answer, we found the index!
                if (strcmp(line, curr_entry.options[i]) == 0)
                {
                    answer_idx = i;
                }
            }

            curr_entry.answer_idx = answer_idx;

            // 🚨 Error checking: If we can't find the answer for the question, throw an error. We're not going to use
            // that question in read_questions.
            if (answer_idx == -1)
            {
                fprintf(
                    stderr,
                    "Error while reading questions: Correct answer \"%s\" for question \"%s\" does not exist in options list.\n\n",
                    line,
                    curr_entry.prompt
                );
            }
            else
            {
                // Else, upload that Entry into the array.
                arr[num_questions] = curr_entry;
                num_questions += 1;
            }
        }
        else if (question_position == LINE_SEPERATOR)
        {
            // This just means we're transitioning to another question.
            // In that case, reset the struct. Since these are passed by value, we'll mem-set them back to 0.
            memset(&curr_entry, 0, sizeof(curr_entry));
        }

        // Update question position
        question_position = (question_position + 1) % 4;
    }
    
    fclose(question_file);

    // Free line if it was malloc'd
    if (line)
    {
        free(line);
    }

    return num_questions;
}

/**
 * Broadcasts a message to all players
 * @param client_players The client players array
 * @param MAX_CONNECTIONS The max number of the connections
 * @param message The message to send
 * @throw Any errors that come up with sending to client socket
*/
void broadcast(struct Player* client_players, int MAX_CONNECTIONS, char* message)
{
    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {
        // Loop through all our valid client socket file descriptors 
        if (client_players[i].fd != -1)
        {
            // Write to the player
            send(client_players[i].fd, message, strlen(message), 0);
        }
    }
}

/**
 * Finds the next available index to store your connection file descriptors in
 * @param client_players The client players array
 * @param max_connections The maximum # of connections. AKA size of client_players
 * @returns The next available index to store connections in. If none exist, return -1.
*/
int find_avail_conn(struct Player* client_players, int max_connections)
{
    for (int i = 0; i < max_connections; i++)
    {
        if (client_players[i].fd == -1)
        {
            return i;
        }
    }

    return -1;
}

/**
 * Given a pointer to a player object, reset that player object.
 * This means everything gets cleared, and its fd gets set to -1.
 * @param player_addr Pointer to player struct
*/
void reset_player(struct Player* player_addr)
{
    memset(player_addr, 0, sizeof(*player_addr));
    player_addr->fd = -1;
}

/**
 * Prints a string using figlet. Of course, we are centering this
 * @param str String to print with figlet
*/
void print_figlet(char* str)
{
    char command[1024] = "figlet -c ";
    strcat(command, str);

    system(command);
}

/**
 * Print the next question in the question list (if it exists)
 * @param questions The question struct list
 * @param curr_question The index of the current question
 * @param max_question The number of questions there are
 * @returns 0 if successful, -1 if not (ran out of questions)
*/
int print_next_question(struct Entry* questions, int curr_question, int max_questions)
{
    // If we ran out of questions, return -1.
    if (curr_question == max_questions)
    {
        return -1;
    }

    // Print in figlet "Question X"
    char qnum[256];
    sprintf(qnum, "Question %d", curr_question + 1);
    print_figlet(qnum);

    // Now, print the question!
    printf("Question %d: %s\n", curr_question + 1, questions[curr_question].prompt);

    // Now, print the possible answers. We know there are only 3 answers to each question.
    for (int i = 0; i < 3; i++)
    {
        printf("%d: %s\n", i + 1, questions[curr_question].options[i]);
    }
    
    return 0;
}

/**
 * Sends the next question to all given players
 * @param client_players The client players array
 * @param max_connections The maximum number of connect
 * @param questions The question struct list
 * @param curr_question The index of the current question
 * @param max_question The number of questions there are
 * @returns 0 if successful. -1 if not (ran out of questions)
*/
int send_next_question(struct Player* client_players, int max_connections, struct Entry* questions, int curr_question, int max_questions)
{
    // If we ran out of questions, return -1.
    if (curr_question == max_questions)
    {
        return -1;
    }

    // Otherwise, format the question string and broadcast it to all players
    char question_str[2048];
    sprintf(question_str, "-----------\nQuestion %d: %s\n", curr_question + 1, questions[curr_question].prompt);

    // Again, print the possible answers. We know there are only 3 answers to each question.
    for (int i = 0; i < 3; i++)
    {
        char curr_qstr[256];
        sprintf(curr_qstr, "%d: %s\n", i + 1, questions[curr_question].options[i]);
        strcat(question_str, curr_qstr);        // Concatenate the stuff we have to question_str
    }

    strcat(question_str, "-----------\n");

    // Now, broadcast it
    broadcast(client_players, max_connections, question_str);

    return 0;
}

/**
 * Print all the player scores so far
*/
void print_player_scores(struct Player* client_players, int max_connections)
{
    puts("--------------------------");
    puts("Current Scores: ");

    for (int i = 0; i < max_connections; i++)
    {
        // Loop through all valid players
        if (client_players[i].fd != -1)
        {
            printf("[%s]: %d\n", client_players[i].name, client_players[i].score);
        }
    }

    puts("--------------------------");
}

/**
 * Prints the winner of the game.
 * If there's a tie, then declare it's a tie.
*/
void print_winner(struct Player* client_players, int max_connections)
{
    // It's impossible for a player to get -999 points. So, that'll me our minimum.
    int max_score = -999;
    char* curr_winner = "No one";
    int tie = 0;                    // Track whether or not we have a tie

    for (int i = 0; i < max_connections; i++)
    {
        // Loop through all valid players and check their score
        if (client_players[i].fd != -1)
        {
            if (client_players[i].score > max_score)
            {
                max_score = client_players[i].score;
                curr_winner = client_players[i].name;
                tie = 0;        // No more tie, we got a new high score
            }
            else if (client_players[i].score == max_score)
            {
                tie = 1;
            }
        }
    }

    // If there's a tie, print that we have a tie.
    if (tie == 1)
    {
        print_figlet("We have a tie!");
    }
    else if (max_score == -999)              // If the max score is still -999, there's no players
    {
        print_figlet("No one wins!");
    }
    else
    {
        // Else, congratulate the winner!
        char win_text[528];
        sprintf(win_text, "Congrats, %s!", curr_winner);

        print_figlet(win_text);
    }
}

/**
 * Destroy the server file descriptor and the client file descriptors. It'll broadcast the closing to all players.
 * Then, exit success.
 * @param client_players The client players array
 * @param MAX_CONNECTIONS The max number of connections
 * @param server_fd 
 * @throw Any errors that come with closing the file descriptors
*/
void destroy_connections(struct Player* client_players, int MAX_CONNECTIONS, int server_fd)
{
    char* message = "Thanks for playing! Good bye!";

    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {
        // Loop through all our valid client socket file descriptors 
        if (client_players[i].fd != -1)
        {
            send(client_players[i].fd, message, strlen(message), 0);
            
            // Close the client socket
            close(client_players[i].fd);
        }
    }

    // Close the server socket
    close(server_fd);

    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv)
{
    // ⭐ Parse command line arguments using getopt()
    int opt;

    // Let's set up the default server parameters
    char question_file_path[1028] = "question.txt";
    char ip_addr[16] = "127.0.0.1";
    int port = 25555;
    int MAX_CONNECTIONS = 3;

    // Loop through all possible opt-strings
    opterr = 0; // Prevent getopt() from printing its own error message.
    while ((opt = getopt(argc, argv, "f:i:p:h")) != -1)
    {
        switch(opt)
        {
            // Question file
            case 'f':
                // We're using strcpy because I do not want to deal with strdup malloc'ing for me.
                strcpy(question_file_path, optarg);
                question_file_path[strlen(optarg)] = '\0';
            break;
            
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
                printf("Usage: %s [-f question file] [-i IP_address] [-p port_number] [-h]\n\n", argv[0]);
                puts("  -f question_file        Default to \"question.txt\";");
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

    // ⭐ Read the questions we have in question.txt. Create variables to handle that
    struct Entry questions[50];
    int num_questions = read_questions(questions, question_file_path);
    int curr_question = 0;
    int game_started = 0;

    // Debug: testing if questions are displayed correctly
    // for (int i = 0; i < num_questions; i++)
    // {
    //     puts(questions[i].prompt);
    //     for (int j = 0; j < 3; j++)
    //     {
    //         puts(questions[i].options[j]);
    //     }
    //     printf("%d\n", questions[i].answer_idx);
    //     puts("---");
    // }

    // ⭐ Create the server
    int server_fd = create_server(ip_addr, port, MAX_CONNECTIONS);

    // Now, we can write server code to accept new players and display questions.
    // Alot of this will be taken from ShuDong's code he went over in class
    // Here, we just declare some basic variables to work with
    fd_set monitoring_set;
    int max_fd = server_fd;             // The maximum file descriptor in our FDT
    int num_connections = 0;        

    // List of all client file descriptors.
    // 💥 This will also be wrapped inside the client_players class for save of convienence
    struct Player client_players[MAX_CONNECTIONS];
    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {
        reset_player(&client_players[i]);
    }

    // Variables to use later when accepting connections
    int recvbytes = 0;
    char buffer[1024];

    // Loop through server code now
    while (1)
    {
        // FD_ZERO to prevent unconditional jump
        FD_ZERO(&monitoring_set);

        // 💥 Monitor all the sockets (parent and child)
        FD_SET(server_fd, &monitoring_set);

        for (int i = 0; i < MAX_CONNECTIONS; i++)
        {
            if (client_players[i].fd != -1)
            {
                FD_SET(client_players[i].fd, &monitoring_set);
            }

            // Also try to update max_fd
            if (client_players[i].fd > max_fd)
            {
                max_fd = client_players[i].fd;
            }
        }

        // Use select to monitor all the sockets for readable fds
        select(max_fd + 1, &monitoring_set, NULL, NULL, NULL);

        // 💥 Server-side code - usually this indicates that a client connection has happened
        if (FD_ISSET(server_fd, &monitoring_set))
        {
            // Before we start, establish a few variables to populate when a request comes in
            struct sockaddr_in in_addr;
            socklen_t addr_size = sizeof(in_addr);

            // First, try to accept the child file descriptors
            int client_fd = accept(server_fd, (struct sockaddr *)&in_addr, &addr_size);

            // 🚨 Error checking: Accepting Server requests not working
            if (client_fd == -1)
            {
                perror("Error when accepting connection");
                continue;
            }

            // Now, check the connections
            if (num_connections < MAX_CONNECTIONS)
            {
                int next_conn_idx = find_avail_conn(client_players, MAX_CONNECTIONS);

                // 🚨 Error checking: # of connections not working
                if (next_conn_idx == -1)
                {
                    fprintf(stderr, "Error when connecting: Ran out of client connection slots.\n");
                    continue;
                }

                client_players[next_conn_idx].fd = client_fd;
                num_connections += 1;
                puts("New connection detected!");

                // Trigger player insert username and stuff.
                char* nameText = "Please type your name: ";
                send(client_fd, nameText, strlen(nameText), 0);
                continue;
            }
            else
            {
                // If we're full, notify the server and close the screen.
                puts("Max connections reached!");
                close(client_fd);
            }
        }

        // 💥 Client-Side code - listen to input from other players. Start reading from the sockets
        for (int i = 0; i < MAX_CONNECTIONS; i++)
        {
            // Loop through all our valid file descriptors and check for changes
            if (client_players[i].fd != -1 && FD_ISSET(client_players[i].fd, &monitoring_set))
            {
                // Read the socket
                recvbytes = read(client_players[i].fd, buffer, sizeof(buffer));

                // If we actually read something
                if (recvbytes > 0)
                {
                    // Put a null terminator at the end so we can nicely parse the input since read() doesn't actually add \0 for you
                    buffer[recvbytes - 1] = '\0';

                    // puts(buffer);

                    // Handle name - CHECK THIS BEFORE ANYTHING
                    if (strcmp(client_players[i].name, "") == 0)
                    {
                        strcpy(client_players[i].name, buffer);
                        printf("Hi %s!\n", buffer);

                        // 💥 If all players have names and the game has not already started, start it
                        if (game_started == 0)
                        {
                            int all_players_have_name = 1;

                            for (int i = 0; i < MAX_CONNECTIONS; i++)
                            {
                                // If there are players with no names, RIP. 
                                if (client_players[i].fd != -1 && strcmp(client_players[i].name, "") == 0)
                                {
                                    all_players_have_name = 0;
                                }
                            }

                            if (all_players_have_name == 1)
                            {
                                game_started = 1;
                                
                                // 💥 Print the game is starting
                                broadcast(
                                    client_players,
                                    MAX_CONNECTIONS,
                                    "The game is starting! Input your answer below to the question on the screen."
                                );

                                // 🚨 Error Checking: If we ran out of questions
                                if (
                                    print_next_question(questions, curr_question, num_questions) == -1 ||
                                    send_next_question(client_players, MAX_CONNECTIONS, questions, curr_question, MAX_CONNECTIONS)
                                )
                                {
                                    puts("Unfortunately, there are no questions. No one wins. :(");
                                    destroy_connections(client_players, MAX_CONNECTIONS, server_fd);
                                }
                            }
                            else
                            {
                                char* response_text = "Welcome! We'll get started momentarily.";
                                send(client_players[i].fd, response_text, strlen(response_text), 0);
                            }
                        }
                        else
                        {
                            char* response_text = "Welcome! The trivia has already started, but you can jump in after the next question!";
                            send(client_players[i].fd, response_text, strlen(response_text), 0);
                        }
                    }
                    else if (game_started == 1)
                    {
                        // Here, the game has begun.

                        // 💥 Check if they had the right answer
                        int player_answer = atoi(buffer) - 1;       // -1 because indexing is weird
                        int correct_answer = questions[curr_question].answer_idx;

                        if (player_answer == correct_answer)
                        {
                            // If the player got it right - congrats! They got the score!
                            client_players[i].score += 1;

                            char* correct_text = "✅ That is correct!";
                            send(client_players[i].fd, correct_text, strlen(correct_text), 0);

                            // Print the correct answer to the screen.
                            char correct_text2[1024];
                            sprintf(
                                correct_text2,
                                "✅ %s got the correct answer! The answer was: \"%s\"!", 
                                client_players[i].name,
                                questions[curr_question].options[correct_answer]
                            );

                            puts(correct_text2);

                            // Also, broadcast it
                            broadcast(client_players, MAX_CONNECTIONS, correct_text2);
                        }
                        else
                        {
                            // If the player got it wrong - they ruined it for everyone L bozo
                            client_players[i].score -= 1;

                            char* wrong_text = "❌ That is wrong!";
                            send(client_players[i].fd, wrong_text, strlen(wrong_text), 0);
                            
                            // Print the correct answer to the screen.
                            char wrong_text2[1024];
                            sprintf(
                                wrong_text2,
                                "❌ %s got the wrong answer! The answer was: \"%s\"!", 
                                client_players[i].name,
                                questions[curr_question].options[correct_answer]
                            );

                            puts(wrong_text2);

                            // Also, broadcast it
                            broadcast(client_players, MAX_CONNECTIONS, wrong_text2);
                        }

                        // Print the player scores
                        print_player_scores(client_players, MAX_CONNECTIONS);

                        // Go to next question
                        curr_question += 1;

                        // 🚨 Error Checking: If we ran out of questions
                        if (
                            print_next_question(questions, curr_question, num_questions) == -1 ||
                            send_next_question(client_players, MAX_CONNECTIONS, questions, curr_question, num_questions) == -1
                        )
                        {
                            print_winner(client_players, MAX_CONNECTIONS);
                            destroy_connections(client_players, MAX_CONNECTIONS, server_fd);
                        }
                    }
                    else
                    {
                        // Game has not started but someone is spamming names
                        char* to_send = "Please be patient! The game has not started yet!";
                        send(client_players[i].fd, to_send, strlen(to_send), 0);
                    }
                }
                else if (recvbytes == 0)            // EOF - Client Disconnected
                {
                    // If we get an EOF here, that means the connection is lost.
                    // Update # connections and destroy/reset all client data
                    num_connections -= 1;

                    close(client_players[i].fd);
                    
                    // Reset the struct
                    reset_player(&client_players[i]);

                    puts("Lost connection!");
                }
                else                                
                {
                    // If we get here, read() has error'd
                    perror("Error while reading client data:");
                }
            }
        }
    }

    // Closes the server in the end
    destroy_connections(client_players, MAX_CONNECTIONS, server_fd);

    return 0;
}