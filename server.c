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

int readPid = 0;
int writePid = 0;

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
	kill( readPid, SIGTERM );
	kill( writePid, SIGTERM );

	wait3 (NULL, WNOHANG, NULL);    
	signal (SIGCHLD, SigCatcher);
}

int main (void) {
	int server_socket, client_socket;
	int client_len;
	struct sockaddr_in client_addr, server_addr;
	
	int bytesRead = 0;
	
	int status = 0;
  
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

	while (1) {
		 // accept a packet from the client
  		client_len = sizeof (client_addr);
		if ((client_socket = accept (server_socket,(struct sockaddr *)&client_addr, &client_len)) < 0) {
			printf ("grrr, can't accept a packet from client\n");
			//close (server_socket);
			return 4;
		}	/* endif */
   

		if ( ( readPid = fork() ) == 0 ) {
			close( server_socket );

			while (1) {
				if ( ( bytesRead = read( client_socket, buffer, BUFSIZ ) ) <= 0 ) {
					if ( bytesRead == 0 ) {
						printf("The client closed the connection.\n");
					}
					else {
						perror("There was an error with the connection.\n");
					}
					break;
				}
				else {
					printf(">>> %s", buffer);
				}

				//readBuffer[0] = 0; // Empty the stringa

				strcpy( buffer, "" );
			}
		}

		else if ( ( writePid = fork() ) == 0 ) {
			close( server_socket );

			while (1) {
				if ( !feof( stdin ) ) {
					fgets( buffer, BUFSIZ, stdin );
				
					if ( write( client_socket, buffer, BUFSIZ ) != -1 ) {
						printf( "<<< %s", buffer );
						fflush( stdout );

						//writeBuffer[0] = 0;
						//strcpy(buffer, "" );
	
					}
					else {
						break;
					}

				}
			}

		}

		close (client_socket);

		waitpid(readPid, &status, WUNTRACED);
		waitpid(writePid, &status, WUNTRACED);

	}

	return 0;
}	/* end main */



