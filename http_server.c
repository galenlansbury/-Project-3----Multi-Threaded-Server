#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

#include "thread_pool.h"
#include "seats.h"
#include "util.h"

#define BUFSIZE 1024
#define FILENAMESIZE 100

void shutdown_server(int);

int listenfd;
pool_t* threadpool; // must make thread pool a global variable for now


int main(int argc,char *argv[])
{
    int flag, num_seats = 20;
    int connfd = 0; //file descriptor for a new connection established by an incoming client
    struct sockaddr_in serv_addr; // ipv4 and 127 and port number and random thing

    char send_buffer[BUFSIZE];
    
    listenfd = 0; // for a socket file desciptor number

    int server_port = 8080;

    if (argc > 1)
    {
        num_seats = atoi(argv[1]); ///deteremines the number of seats
    } 

    if (server_port < 1500)
    {
        fprintf(stderr,"INVALID PORT NUMBER: %d; can't be < 1500\n",server_port);
        exit(-1);
    }
    
    if (signal(SIGINT, shutdown_server) == SIG_ERR)  //if error, shutdown
        printf("Issue registering SIGINT handler");

    listenfd = socket(AF_INET, SOCK_STREAM, 0); //Socket stream is a TCP protocol and IPv4
	///socket system call (this is a file descriptor :The socket system call returns an entry into the servers file descriptor table [for open IO])
    if ( listenfd < 0 ){
        perror("Socket");
        exit(errno);
    }

    printf("Established Socket: %d\n", listenfd);
    flag = 1;
    setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag) ); //another socket system call

    // Load the seats; mallocs in seats.c
    load_seats(num_seats);

    // set server address 
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(send_buffer, '0', sizeof(send_buffer));
    serv_addr.sin_family = AF_INET; //ipv4
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(server_port);

    // bind to socket to specific server IP address, (localhost)
    if ( bind(listenfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) != 0)
    {
        perror("socket--bind");
        exit(errno);
    }

    // listen for incoming requests
    listen(listenfd, 10); //10 is backlog (10 is the amount fo parralel request, waiting queue is length 10)







    // TODO: Initialize your threadpool!

	threadpool = pool_create(10,5000); //@#ofthreads, @#ofItemsInQueue
	// this will create 10 idle queues that will wait for new queues to be added!


    // This while loop "forever", handling incoming connections
    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); //The accept() system call causes the process to block until a client connects to the server
		//It returns a new file descriptor for the incoming connection

		
        /*********************************************************************
            You should not need to modify any of the code above this comment.
            However, you will need to add lines to declare and initialize your 
            threadpool!
			*
            The lines below will need to be modified! Some may need to be moved
            to other locations when you make your server multithreaded.
        *********************************************************************/

		while(pool_add_task(threadpool, (void*)dowork, (void*)connfd)); //DoWork is a sub function that handles parse request and process request

		//if mainThread successfully added this to queue this will return true

        //struct request req;
        // parse_request fills in the req struct object
       // parse_request(connfd, &req);
        // process_request reads the req struct and processes the command
     //   process_request(connfd, &req);
        close(connfd);
    }
}




void shutdown_server(int signo){
    printf("Shutting down the server...\n");
    
    // TODO: Teardown your threadpool

    // TODO: Print stats about your ability to handle requests.
    unload_seats();
    close(listenfd);
    exit(0);
}
