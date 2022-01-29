// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <thread>
#include <iostream>
#include <signal.h>
#include <chrono>

//-----------------------------------------------------------------------------

#define PORT	7776
#define MAXSIZE 1500
#define SERVER  "192.168.11.105"
#define INPUT_ARGUMENTS_MAX 6

//-----------------------------------------------------------------------------

int sockfd;
struct sockaddr_in servaddr;
int packet_send, packet_recv;
int buffer_size = MAXSIZE;  // length of udp packet
int packet_count = 1;       // number of packets
int timeout = 0;

//-----------------------------------------------------------------------------

void sigintHandler(int signum)
{
   printf("Send - %d, recv - %d\n", packet_send, packet_recv);
   exit (1);
}

//-----------------------------------------------------------------------------

unsigned int get_flags(int argc, char ** argv)
{
    unsigned int arg_bit = 0;
    char * input_arg[INPUT_ARGUMENTS_MAX] =
    {
        "-p", "-l", "-s", "-d", "-t", "-h"
    };

    // become all possible arguments
    for (int i = 1; i < argc; i++)
    {
        for (int j = 0; j < INPUT_ARGUMENTS_MAX; j++)
        {
            if (!strcmp(argv[i], input_arg[j]))
            {
                if (j == 0)
                {
                    // argument -p
                    arg_bit |= 1;
                }
                else if (j == 1)
                {
                    // argument -l
                    arg_bit |= 2;
                }
                else if (j == 2)
                {
                    // argument -s
                    arg_bit |= 4;
                }
                else if (j == 3)
                {
                    // argument -d
                    arg_bit |= 8;
                }
                else if (j == 4)
                {
                    // argument -t
                    arg_bit |= 16;
                }
                else if (j == 5)
                {
                    // argument -h
                    arg_bit = 425;
                    break;
                }
            }
        }
    }

    return arg_bit;
}

//-----------------------------------------------------------------------------

void help_message()
{
    printf(" Udp echo-client (2021 October 5)   \n\n"
           " Arguments:                         \n"
           "    -p         Specify port         \n"
           "    -l         Specify host ip      \n"
           "    -s         Specify buffer size  \n"
           "    -d         Specify number of packets \n"
           "    -t         Specify timeout in nanoseconds \n"
           "    -h         Show help message    \n\n");
    exit(EXIT_SUCCESS);
}

//-----------------------------------------------------------------------------

void task1()
{
    char buffer[MAXSIZE];
    packet_send = 0;
    int i;

    // Задать текст сообщения.
    for (i = 0; i < buffer_size; i++)
        buffer[i] = i+1;

    auto start = std::chrono::high_resolution_clock::now();

    for (i = 0; i < packet_count; i++)
    {
        sendto(sockfd, (const char *)buffer, buffer_size,
            MSG_CONFIRM, (const struct sockaddr *) &servaddr,
                sizeof(servaddr));
        packet_send++;
        std::this_thread::sleep_for(std::chrono::nanoseconds(timeout));
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end-start;
    std::cout << "Stop sending: " << elapsed.count() / 1000000000 << " s\n";
}

//-----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    int port = PORT;
    int i, n;
    unsigned int len;
    unsigned int flags;
    char buffer[MAXSIZE];
    signal(SIGINT, sigintHandler);

    //-------------------------------------------------------------------------

    // get all arguments
    flags = get_flags(argc, argv);

    //-------------------------------------------------------------------------

    // when flag -h is specified
    if (flags == 425)
        help_message();

    //-------------------------------------------------------------------------

    // Filling default server information
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    if (inet_aton(SERVER , &servaddr.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    //-------------------------------------------------------------------------

    // when flag -p is specified
    if (flags & 1)
    {
        for (int i = 1; i < argc; i++)
        {
           if (!strcmp(argv[i], "-p"))
           {
               ++i;

               if (i == argc)
               {
                   fprintf(stderr, "No port is specified\n"
                                   "Use argument -h for more details\n\n");
                   exit(EXIT_FAILURE);
               }
               else
               {
                   port = atoi(argv[i]);
                   servaddr.sin_port = htons(port);

                   break;
               }
           }
        }
    }

    //-------------------------------------------------------------------------

    // when flag -l is specified
    if ((flags >> 1) & 1)
    {
        for (int i = 1; i < argc; i++)
        {
           if (!strcmp(argv[i], "-l"))
           {
               ++i;

               if (i == argc)
               {
                   fprintf(stderr, "No host is specified\n"
                                   "Use argument -h for more details\n\n");
                   exit(EXIT_FAILURE);
               }
               else
               {
                   // set ip_address
                   if (inet_aton(argv[i] , &servaddr.sin_addr) == 0)
                   {
                       fprintf(stderr, "inet_aton() failed\n");
                       exit(1);
                   }

                   break;
               }
           }
        }
    }

    //-------------------------------------------------------------------------

    // when flag -s is specified
    if ((flags >> 2) & 1)
    {
        for (int i = 1; i < argc; i++)
        {
           if (!strcmp(argv[i], "-s"))
           {
               ++i;

               if (i == argc)
               {
                   fprintf(stderr, "No buffer size is specified\n"
                                   "Use argument -h for more details\n\n");
                   exit(EXIT_FAILURE);
               }
               else
               {
                   buffer_size = atoi(argv[i]);
                   break;
               }
           }
        }
    }

    //-------------------------------------------------------------------------

    // when flag -d is specified
    if ((flags >> 3) & 1)
    {
        for (int i = 1; i < argc; i++)
        {
           if (!strcmp(argv[i], "-d"))
           {
               ++i;

               if (i == argc)
               {
                   fprintf(stderr, "No number of packets is specified\n"
                                   "Use argument -h for more details\n\n");
                   exit(EXIT_FAILURE);
               }
               else
               {
                   packet_count = atoi(argv[i]);
                   break;
               }
           }
        }
    }

    //-------------------------------------------------------------------------

    // when flag -t is specified
    if ((flags >> 4) & 1)
    {
        for (int i = 1; i < argc; i++)
        {
           if (!strcmp(argv[i], "-t"))
           {
               ++i;

               if (i == argc)
               {
                   fprintf(stderr, "No timeout is specified\n"
                                   "Use argument -h for more details\n\n");
                   exit(EXIT_FAILURE);
               }
               else
               {
                   timeout = atoi(argv[i]);
                   break;
               }
           }
        }
    }

    //-------------------------------------------------------------------------

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    //-------------------------------------------------------------------------

    // start thread-sender
    std::thread t1(task1);

    //-------------------------------------------------------------------------

    // Start receive
    while (true)
    {
        n = recvfrom(sockfd, (char *)buffer, strlen(buffer),
                    MSG_WAITALL, (struct sockaddr *) &servaddr,
                    &len);
        packet_recv++;
        //buffer[n] = '\0';
    }

    t1.join();

    //-------------------------------------------------------------------------

    // close socket
    close(sockfd);
    return 0;
}
