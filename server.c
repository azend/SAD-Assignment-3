/*
 * server.c
 *
 * This is a sample internet server application that will respond
 * to requests on port 5000
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#define PORT 5000

char buffer[BUFSIZ];


/*
 * this signal handler is used to catch the termination
 * of the child. Needed so that we can avoid wasting
 * system resources when "zombie" processes are created
 * upon exit of the child (as the parent could concievably
 * wait for the child to exit)
 */

void
SigCatcher (int n)
{
    wait3 (NULL, WNOHANG, NULL);    
	signal (SIGCHLD, SigCatcher);
}

int
main (void)
{
	int server_socket, client_socket, shmid;
	int client_len;
	struct sockaddr_in client_addr, server_addr;
	key_t memShareKey;
    int * q;

    //Generate a memory share key
    memShareKey = ftok (".", 'M');
    
    if (memShareKey == -1) {
        printf("Cannot allocate memory share key");
    }
    
    if ((shmid = shmget (memShareKey, sizeof (int), 0)) == -1){
        shmid = shmget (memShareKey, sizeof (int), IPC_CREAT | 0660);
        if (shmid == -1){
            printf("Cannot allocate new shared memory\n");
        }
    }
    
    q = (int *)shmat (shmid, NULL, 0);
    if (q == NULL) {
        printf("Oh snap, attached memory failed");
    }


	/*
	 * install a signal handler for SIGCHILD (sent when the child terminates)
	 */

	signal (SIGCHLD, SigCatcher);

	/*
	 * obtain a socket for the server
	 */
	if ((server_socket = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		printf ("grrr, can't get the server socket\n");
		return 1;
	}	/* endif */

	/*
	 * initialize our server address info for binding purposes
	 */
	memset (&server_addr, 0, sizeof (server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl (INADDR_ANY);
	server_addr.sin_port = htons (PORT);

	if (bind (server_socket, (struct sockaddr *)&server_addr, 
	sizeof (server_addr)) < 0) {
		printf ("grrr, can't bind server socket\n");
		close (server_socket);
		return 2;
	}	/* endif */

	/*
	 * start listening on the socket
	 */
	if (listen (server_socket, 5) < 0) {
		printf ("grrr, can't listen on socket\n");
		close (server_socket);
		return 3;
	}	/* endif */

	/*
	 * for this server, run an endless loop that will
	 * accept incoming requests from a remote client.
	 * the server will fork a child copy of itself to handle the
	 * request, and the parent will continue to listen for the
	 * next request
	 */
    //set the value of q to zero
    *q = 0;
	while (1) {
		 // accept a packet from the client
  		client_len = sizeof (client_addr);
		if ((client_socket = accept (server_socket,(struct sockaddr *)&client_addr, &client_len)) < 0) {
			printf ("grrr, can't accept a packet from client\n");
			close (server_socket);
			return 4;
		}	/* endif */
    
        //read messages from the client

        if (buffer[0] != '\0'){
            if (fork() == 0) {
                printf("%s\n",buffer);
                buffer[0] = '\0';
                close (client_socket);
            }
            buffer[0] = '\0';
        }
        
        // Send a message to the client
        if (*q == 0){
            *q = 1; // setting q to one will prevent a new fork from launching before the old fork is finished
            if (fork() == 0) {
                buffer[0] = '\0';
                printf("Type a message: ");
                fflush (stdout);
                fgets (buffer, sizeof(buffer), stdin);
                if (buffer[strlen (buffer) - 1] == '\n') {
                    buffer[strlen (buffer) - 1] = '\0';
                }
                write (client_socket, buffer, strlen (buffer));
                buffer[0] = '\0';
                close (client_socket);
                *q = 0;
            }
		}
        read (client_socket, buffer, sizeof (buffer));

	}	/* end while */
    close (client_socket);
    shmdt (q);
    shmctl (shmid, IPC_RMID, 0);
	return 0;
}	/* end main */



