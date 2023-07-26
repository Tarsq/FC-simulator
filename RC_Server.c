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
	char buff[MAXLEN], answer[MAXLEN];
	int n, addr_length;

	/* Socket address structure */
	struct sockaddr_in socket_addr;
	struct sockaddr client_addr;

	/* Create socket variable */
	int my_socket = socket(AF_INET, SOCK_DGRAM, 0);

	if(my_socket == -1) {
		fprintf(stderr, "Cannot create socket\n");
		return 0;
	}

	/* Initialize socket address to 0*/
	memset(&socket_addr, 0, sizeof(socket_addr));
	/* Set socket address parameters */
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_port = htons(1100);
	socket_addr.sin_addr.s_addr = INADDR_ANY;

	/* Bind socket to socket address struct */
	if(bind(my_socket, (struct sockaddr *)&socket_addr, sizeof(socket_addr)) == -1) {
		fprintf(stderr, "Cannot bind socket\n");
		close(my_socket);
		return 0;
	}
	addr_length = sizeof(client_addr);
	printf("I'm waiting.\n");

	int pitch;
	int roll;
	int yow;
	char signal_strength = 100;

	for(;;) {

		/* Read what others want to tell you */    
		n = recvfrom(my_socket, (char *)buff, MAXLEN, MSG_WAITALL, &client_addr, &addr_length); 
		pitch = buff[0];
		roll = buff[1];
		yow = buff[2];
		printf("pitch: %d\n roll: %d\n yow: %d\n",pitch,roll, yow );

		/* Prepare the answer and send it back */
		sprintf(answer, "%s", &signal_strength);
    	sendto(my_socket, answer, strlen(answer), MSG_CONFIRM, &client_addr, addr_length); 
	}

	return EXIT_SUCCESS;
}
