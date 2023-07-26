#include "Gimbals_position.h"

/* Thread variable */
pthread_t tRcThread;
/* Thread attributes */
pthread_attr_t aRcThreadAttr;

/* Socket address structure */
struct sockaddr_in socket_addr;
//int answer;
void *tRcThreadFunc(void *cookie);

/* Screen thread initialization function */
int init_RC() {
    int status;
    /* Create thread for Gimbals position receiver */
    pthread_attr_init(&aRcThreadAttr);
    pthread_attr_setschedpolicy(&aRcThreadAttr, SCHED_FIFO);

    if (status = pthread_create(&tRcThread, &aRcThreadAttr,tRcThreadFunc,NULL)){
        fprintf(stderr, "Cannot create thread. \n");
        return 1;
    }    

    /* Initialize socket address*/
	memset(&socket_addr, 0, sizeof(socket_addr));
	/* Set socket address parameters */
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_port = htons(1110);
	socket_addr.sin_addr.s_addr = INADDR_ANY;

	return 0;
}

/* GIMBAL POSITION RECEIVER - server */
void *tRcThreadFunc(void *cookie) {
    //printf("START Gmibal server\n");
	int i = 0;

	int policy;/* Scheduling policy: FIFO or RR */
	struct sched_param param;/* Structure of other thread parameters */

	pthread_getschedparam( pthread_self(), &policy, &param); /* Read modify and set new thread priority */
	param.sched_priority = sched_get_priority_max(policy)-2;
	pthread_setschedparam( pthread_self(), policy, &param);

    // START Server
	int16_t buff[4];
    char answer[MAXLEN];
	int n, addr_length;

	/* Socket address structure */
	struct sockaddr_in socket_addr;
	struct sockaddr client_addr;

	/* Create socket variable */
	int my_socket = socket(AF_INET, SOCK_DGRAM, 0);

	if(my_socket == -1) {
		fprintf(stderr, "Cannot create socket gimbal\n");
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
		fprintf(stderr, "Cannot bind socket gimbal\n");
		close(my_socket);
		return 0;
	}
	addr_length = sizeof(client_addr);
 
	for(;;) {
		//printf("blah from gimbals server \n");
		n = recvfrom(my_socket, (short *)buff, MAXLEN, MSG_WAITALL, &client_addr, &addr_length);
	
		pthread_mutex_lock(&mutex_pitch);
		Pitch_PID.set_point = buff[0];
		pthread_mutex_unlock(&mutex_pitch);

		pthread_mutex_lock(&mutex_roll);
		Roll_PID.set_point = buff[1];
		pthread_mutex_unlock(&mutex_roll);
		
		pthread_mutex_lock(&mutex_yaw);
		Yaw_PID.set_point = buff[2];
		pthread_mutex_unlock(&mutex_yaw);

		pthread_mutex_lock(&mutex_throttle);
		throttle = buff[3];
		pthread_mutex_unlock(&mutex_throttle);
		
		char signal_strength = check_RSSI();
		/* Prepare the answer and send it back */
		sprintf(answer, "%s", &signal_strength);
		sendto(my_socket, answer, strlen(answer), MSG_CONFIRM, &client_addr, addr_length); 
	}

	return 0;
}

int check_RSSI()
{
	int RSSI_value;
	RSSI_value = (int)(rand()%20)+80; 
	return RSSI_value;
}