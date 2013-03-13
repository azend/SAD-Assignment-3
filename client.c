/*
 * client.c
 *
 * This is a sample internet client application that will talk
 * to the server s.c via port 5000
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
#include <errno.h>

#define PORT 5000

char buffer[BUFSIZ];

int
main (int argc, char *argv[])
{
	int client_socket, len, shmid;
	struct sockaddr_in server_addr;
	struct hostent *host;
    key_t memShareKey;
    int * p;
    
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
    
    p = (int *)shmat (shmid, NULL, 0);
    if (p == NULL) {
        printf("Oh snap, attached memory failed");
    }
	/*
	 * check for sanity
	 */

	if (argc != 2) {
		printf ("usage: c server_name\n");
		return 1;
	}	/* endif */

	/*
	 * determine host info for server name supplied
	 */

	if ((host = gethostbyname (argv[1])) == NULL) {
		printf ("grrr, can't get host info!\n");
		return 2;
	}	/* endif */

	/*
	 * get a socket for communications
	 */

	if ((client_socket = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		printf ("grrr, can't get a client socket!\n");
		return 3;
	}	/* endif */

	/*
	 * initialize struct to get a socket to host
	 */
 
	memset (&server_addr, 0, sizeof (server_addr));
	server_addr.sin_family = AF_INET;
	memcpy (&server_addr.sin_addr, host->h_addr, host->h_length);
	server_addr.sin_port = htons (PORT);

	/*
	 * attempt a connection to server
	 */
  
	if (connect (client_socket, (struct sockaddr *)&server_addr,
	sizeof (server_addr)) < 0) {
		printf ("grrr, can't connet to server!\n");
		close (client_socket);
		return 4;
	}	/* endif */

	/*
	 * now that we have a connection, get a commandline from
	 * the user, and fire it off to the server
	 */
     printf("5 I made it to here\n"); 
    //recieve a message from the server
    *p = 0;
    while (1) {
        
                
        // If a message is recieved, print it and zero the buffer
        if (buffer[0] != '\0'){
            if (fork() == 0){
                printf ("%s\n", buffer);
                buffer[0] = '\0';
                close (client_socket);
            }
            buffer[0] = '\0';
        }
        
        // send a message to the server
        if (*p == 0){
            *p = 1;
            if (fork() == 0){
                buffer[0] = '\0';
                printf("Type a message: ");
                fflush (stdout);
                fgets (buffer, sizeof (buffer), stdin);
                if (buffer[strlen (buffer) - 1] == '\n'){
                    buffer[strlen (buffer) - 1] = '\0';
                }
                strcat (buffer, "\0");
                write (client_socket, buffer, strlen (buffer));
                buffer[0] = '\0';
                close (client_socket);
                *p = 0;
            }
        }
        read (client_socket, buffer, sizeof (buffer));

    } // end while


    /*
    * cleanup
    */
    close (client_socket);
    shmdt (p);
    shmctl (shmid, IPC_RMID, 0);
	return 0;
}	/* end main */



