#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXLEN 100

int main(void) {

	char buff[MAXLEN]; 
	int n, addr_length;
	/* Socket address structure */
	struct sockaddr_in socket_addr;
	/* Create socket */
	int my_socket = socket(PF_INET, SOCK_DGRAM, 0);

	if (my_socket == -1) {
		fprintf(stderr, "Cannot create socket\n");
		return 0;
	}

	/* Initialize socket address to 0*/
	memset(&socket_addr, 0, sizeof(socket_addr));
	/* Set socket address parameters */
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_port = htons(1100);
	socket_addr.sin_addr.s_addr = INADDR_ANY;

	int c;
	// from -32,768 to 32,767
	short angle[3];
	angle[0] = 670;
	angle[1] = 0;
	angle[2] = 0;
		
	/* Send a message to server */
	sendto(my_socket, angle, sizeof(angle), MSG_CONFIRM, (const struct sockaddr *) &socket_addr, sizeof(socket_addr)); 
	
	/* Read the reply from server */      
	n = recvfrom(my_socket, (char *)buff, MAXLEN, MSG_WAITALL, (struct sockaddr *) &socket_addr, &addr_length); 
	printf("Signal strength : %d %%\n", *buff); 
	/* Clean up */
	close(my_socket); 
	
	return EXIT_SUCCESS;
}
