#include <sys/time.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <limits.h>
#include <sys/types.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include "mystack.h"
#include <stdio.h>

#define PORT 5060          // port for the sender
#define MAYA 5251          // first id
#define YOGEV 9881         // second id
#define CON_IN_STK 100     // how many pending connections Stack will hold
#define fileSize 1462544   // 1 MB
#define BUFFERSIZE 1462544 // 1 MB
#define receiverPORT 5060  // port for the receiver
#define ONE 1

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

char message[BUFFERSIZE];                                   // the message we will send
void ConnectionSteps(int senderSocket, int receiverSocket); // function who deals with the connection
int SendFileData(int number, int fd);                       //
void PrintTimeData(int number_of_iterations);

int main(int argc, char **argv)
{
    int receiverSocket;               // the socket of the receiver
    int senderSocket;                 // the socket of the sender
    int addrSize;                     //
    SA_IN receiver_addr, sender_addr; // the address of the receiver and the sender

    if ((receiverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) // create the socket
    {
        perror("oops, socket failed"); // if the socket failed
        exit(ONE);
    }
    else // if the socket created successfully
    {
        printf("OMG!the socket created successfully\n");
    }

    // initialize the adress struct
    receiver_addr.sin_family = AF_INET;           // host byte order
    receiver_addr.sin_addr.s_addr = INADDR_ANY;   // automatically fill with my IP
    receiver_addr.sin_port = htons(receiverPORT); // short, network byte order

    // bind the socket to the port
    if (bind(receiverSocket, (SA *)&receiver_addr, sizeof(receiver_addr)) == -1)
    {
        perror("oops, bind failed, you should try again"); // if the bind failed
        exit(ONE);
    }
    // listen to the socket
    if (listen(receiverSocket, CON_IN_STK) == -1) // if the listen failed
    {
        perror("oops, listen failed, you should try again");
        exit(ONE); // if the listen failed
    }

    while (1)
    {
        printf("the socket is waiting for connections... \n");
        // wait for and eventually accept an incomming connection

        addrSize = sizeof(SA_IN);
        if ((senderSocket = accept(receiverSocket, (SA *)&sender_addr, (socklen_t *)&addrSize)) == -1) // if the accept failed
        {

            printf("socket has been closed, exiting the program\n");
            printf("hope to see you next time !<3 \n -maya and yogev -\n");

            exit(ONE); // if the accept failed
        }
        printf("connected succesfully! on socket %d \n", senderSocket); // if the accept succeeded

        // the the thing we want to do with the connection
        ConnectionSteps(senderSocket, receiverSocket);
    }
    close(receiverSocket); // close the socket
    printf("closing the receiver socket");
}

void ConnectionSteps(int senderSocket, int receiverSocket) // function who deals with the connection
{
    // recieve the first part of the our_file

    uint32_t XOR = MAYA ^ YOGEV; // the XOR of  maya and yogev id's
    int interationCounter = 0;
    int numOfBytes = 0;

    while (ONE) // while the connection is alive
    {
        struct timeval start_t_cubic, end_t_cubic, timeResultCubic;
        struct timeval startReno, endReno, timeResultReno;

        interationCounter++;
        bzero(message, BUFFERSIZE);

        // set the algorithm to cubic
        char *cc = "cubic";
        if (setsockopt(senderSocket, IPPROTO_TCP, TCP_CONGESTION, cc, strlen(cc)) > 0 || setsockopt(senderSocket, IPPROTO_TCP, TCP_CONGESTION, cc, strlen(cc)) < 0)
        {
            printf("setsockopt has been failed \n"); // if the setsockopt failed
            return;                                  // exit the function
        }
        printf("CC set to %s\n", cc);

        gettimeofday(&start_t_cubic, NULL); // starts the time

        for (; numOfBytes < BUFFERSIZE / 2; numOfBytes++)
        {
            if (!(recv(senderSocket, message, 1, 0) != -1))
            {
                printf("receive has been failed \n");
                break;
            }
        }
        usleep(1000);                     // sleep for 1 second
        gettimeofday(&end_t_cubic, NULL); // finish count for first part of the our_file

        bzero(message, BUFFERSIZE);                               // clear the buffer
        timersub(&end_t_cubic, &start_t_cubic, &timeResultCubic); // the total time cubic

        long int *timeElapseCubic = (long int *)malloc(sizeof(long int));
        *timeElapseCubic = timeResultCubic.tv_sec * 1000000 + timeResultCubic.tv_usec;
        int *interationNumberPoint = (int *)malloc(sizeof(int));
        *interationNumberPoint = interationCounter;
        int *cubicParam = (int *)malloc(sizeof(int));
        *cubicParam = 0;
        push(timeElapseCubic, interationNumberPoint, cubicParam); // push the data to the list

        printf("algo: cubic, time: %ld.%06ld, iter number: %d\n",
               (long int)timeResultCubic.tv_sec,
               (long int)timeResultCubic.tv_usec,
               interationCounter); // print the data

        // Send back the authentication to the sender.
        // send thhe XOR to the sender
        printf("sending the XOR to the sender %d \n", XOR); // print the XOR
        SendFileData(XOR, senderSocket);

        bzero(message, BUFFERSIZE);

        // change the algorithm to reno
        char *ccAlgorithm = "reno"; // the CC algorithm to use (in this case, "reno")
        if (setsockopt(receiverSocket, IPPROTO_TCP, TCP_CONGESTION, ccAlgorithm, strlen(ccAlgorithm)))
        {
            perror("setsockopt failed"); // if the setsockopt failed
            exit(1);                     // exit the program
        }

        printf("CC set to %s\n", ccAlgorithm); // print the CC algorithm

        // recive the second part of the our_file
        gettimeofday(&startReno, NULL); // start the time

        // recive a our_file of half mega bytes
        for  ( ; numOfBytes < BUFFERSIZE; numOfBytes++)
        {
            int result = recv(senderSocket, message, 1, 0);
            if (!(result != -1))
            {
                printf("oops, recive failed... you should try again \n");
                break;
            }
        }
        usleep(1000);
        gettimeofday(&endReno, NULL);                    // finish count for first part of the our_file
        timersub(&endReno, &startReno, &timeResultReno); // the total time reno

        printf("algo: reno, time: %ld.%06ld, iter number: %d\n", (long int)timeResultReno.tv_sec, (long int)timeResultReno.tv_usec, interationCounter);

        // store the time elapsed in a variable
        long int timeElapseReno = timeResultReno.tv_sec * 1000000 + timeResultReno.tv_usec;
        long int *timeElapseRenoP = (long int *)malloc(sizeof(long int));
        *timeElapseRenoP = timeElapseReno;
        int *renoParam = (int *)malloc(sizeof(int));
        *renoParam = 1;

        push(timeElapseRenoP, interationNumberPoint, renoParam); // push the data to the Stack

        // if you get the exit message from the sender, close the socket and exit
        recv(senderSocket, message, 1024, 0);
        // if the sender send the message "again" the receiver will recive the our_file again
        if (!(strcmp(message, "again") != 0))
        {
            continue; // recive the our_file again
        }
        else
        {
            // print out the report
            printf("\n\n");
            printf("the Execution time report \n");
            PrintTimeData(interationCounter);
            close(senderSocket);
            close(receiverSocket);
            printf("closing the sender socket \n");
            printf("closing the receiver socket \n");
            return;
        }
    }
}

int SendFileData(int number, int fd) // sending the our_file data
{

    int32_t conv = htonl(number); // convert the number to network byte order
    int total_sent = 0;           // counting the total bytes sent
    char *data = (char *)&conv;   // the data to send
    int sent;

    // send the number to the sender
    while (total_sent < sizeof(conv))
    {
        sent = write(fd, data + total_sent, sizeof(conv) - total_sent);
        if (sent < 0) // if the sent is less than 0
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) // if the socket is not ready
            {
                printf(" sorry, there is a problem with sending");
                return -1;
            }
            else if (errno != EINTR) // if the socket is not ready
            {
                printf(" sorry, there is a problem with sending");
                return -1; // return -1
            }
        }
        else if (!(sent != 0))
        {
            printf("we didn't send anything"); // if we didn't send anything
            return -1;
        }
        else // if we sent something
        {
            total_sent = total_sent + sent;
        }
    }

    return 0;
}

void PrintTimeData(int number_of_iterations)
{
    long int averageC = 0;     // the average time for cubic
    long int averageR = 0;     // the average time for reno
    long int averageTotal = 0; // the average time for total
    long int numberOfDeq = 0;  // the number of pop from the stack

    // remove from the stack and print the data
    while (first != NULL) // while the stack is not empty
    {
        if (!(*first->cubic_is_0_reno_is_1 != 0)) // if the algorithm is cubic - add it to the averageC
        {
            averageC += *first->timeInMicroSeconds;
        }
        else if (!(*first->cubic_is_0_reno_is_1 != 1)) // if the algorithm is reno - add it to the averageR
        {
            averageR += *first->timeInMicroSeconds;
        }
        averageTotal += *first->timeInMicroSeconds; // add the time to the total average

        pop();
        numberOfDeq++; // increase the number of pop
    }
    // print out the report
    printf("*** the result report:  **\n");                                                      // print the report
    printf("The average time for cubic is %ld microseconds\n", averageC / number_of_iterations); // print the average time for cubic
    printf("The average time for reno is %ld microseconds\n", averageR / number_of_iterations);  // print the average time for reno
    printf("The average time for total is %ld microseconds\n", averageTotal / numberOfDeq);      // print the average time for total
    printf("The number of iterations is %d \n", number_of_iterations);                           // print the number of iterations
    if (averageC < averageR)                                                                     // print the winner
    {
        printf("the winner is: cubic \n"); // print the winner
    }
    else if (averageC > averageR)
    {
        printf("Congratulations, the winner is: reno \n");
    }
    else
    {
        printf("Congratulations, the winner is: no one, they are equal \n");
    }
}