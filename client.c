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
	int client_socket;
	struct sockaddr_in server_addr;
	struct hostent *host;
    	int readPid = 0;
	int writePid = 0;
	int bytesRead = 0;
	int status = 0;

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
   
		readPid = fork();
		writePid = fork();

		if ( readPid == 0 ) {

			while (1) {
				if ( ( bytesRead = read( client_socket, buffer, BUFSIZ ) ) <= 0 ) {
					if ( bytesRead == 0 ) {
						printf("The client closed the connection.\n");
					}
					else {
						perror("There was an error with the connection.\n");
					}

					close( client_socket );
					break;
				}
				else {
					printf(">>> %s", buffer);
				}

				//readBuffer[0] = 0; // Empty the stringa

				strcpy( buffer, "" );
			}
		}

		else if ( writePid == 0 ) {

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
						close( client_socket );
						break;
					}

				}
			}

		}
		
		else {
			close( client_socket );	
		}

		waitpid(readPid, &status, WUNTRACED);
		waitpid(writePid, &status, WUNTRACED);


    /*
    * cleanup
    */
    close (client_socket);
	return 0;
}	/* end main */



