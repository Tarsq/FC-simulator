#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "model_output.h"
#include "model.h"

/* critical sections */
extern pthread_rwlock_t lock_Modelstate;
extern ModelState Model_state;
extern short pitch_noise, roll_noise;
extern pthread_mutex_t mutex_Noise;

pthread_t tPlantOutputsThread;
pthread_attr_t aPlantOutputsThreadAttr;

pthread_t tWindDisruptionThread;
pthread_attr_t aWindDisruptionThreadAttr;

pthread_barrier_t noise_barrier;

/* Struct for sensor client*/
struct sockaddr_in socket_addr;

int init_ModelOutputs(void)
{
    int status;
    pthread_attr_init(&aPlantOutputsThreadAttr);
	pthread_attr_setschedpolicy(&aPlantOutputsThreadAttr, SCHED_FIFO);
    /* Create PIDs thread */
    if (status = pthread_create(&tPlantOutputsThread, &aPlantOutputsThreadAttr, tPlantOutputsThreadFunc, NULL))
    {
        fprintf(stderr, "Cannot create thread.\n");
        return 0;
    }
	
}

void *tPlantOutputsThreadFunc(void *cookie) {
	int i = 0;

	int policy;/* Scheduling policy: FIFO or RR */
	struct sched_param param;/* Structure of other thread parameters */

	pthread_getschedparam( pthread_self(), &policy, &param); /* Read modify and set new thread priority */
	param.sched_priority = sched_get_priority_max(policy);
	pthread_setschedparam( pthread_self(), policy, &param);

    // START Server
	int n, addr_length;
	char measure_flag[1]; // if 1 start measurement

	/* Socket address structure */
	struct sockaddr_in socket_addr;
	struct sockaddr client_addr;

	/* Create socket variable */
	int my_socket = socket(AF_INET, SOCK_DGRAM, 0);

	if(my_socket == -1) {
		fprintf(stderr, "Cannot create socket model\n");
		return 0;
	}

	/* Initialize socket address to 0*/
	memset(&socket_addr, 0, sizeof(socket_addr));
	/* Set socket address parameters */
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_port = htons(1110);
	socket_addr.sin_addr.s_addr = INADDR_ANY;

	/* Bind socket to socket address struct */
	if(bind(my_socket, (struct sockaddr *)&socket_addr, sizeof(socket_addr)) == -1) {
		fprintf(stderr, "Cannot bind socket model\n");
		close(my_socket);
		return 0;
	}
	addr_length = sizeof(client_addr);
 
	for(;;) {
		n = recvfrom(my_socket, (char *)measure_flag, MAXLEN, MSG_WAITALL, &client_addr, &addr_length);
		if(measure_flag[0] != 1)
			break;

		short buff[3];
		pthread_rwlock_rdlock(&lock_Modelstate);
		buff[0] = Model_state.pitch;
		buff[1] = Model_state.roll;
		buff[2] = Model_state.yaw;
		pthread_rwlock_unlock(&lock_Modelstate);
		sendto(my_socket, buff, sizeof(buff), MSG_CONFIRM, &client_addr, addr_length); 
	}

	return 0;
}

int init_WindDisruption(void)
{
    int status;
	pthread_barrier_init(&noise_barrier, NULL, 2);
    pthread_attr_init(&aWindDisruptionThreadAttr);
	pthread_attr_setschedpolicy(&aWindDisruptionThreadAttr, SCHED_FIFO);
    if (status = pthread_create(&tWindDisruptionThread, &aWindDisruptionThreadAttr, tWindDisruptionThreadFunc, NULL))
    {
        fprintf(stderr, "Cannot create thread.\n");
        return 0;
    }
}

void *tWindDisruptionThreadFunc(void *cookie)
{
    int policy;
	struct sched_param param;

	pthread_getschedparam( pthread_self(), &policy, &param); 
	param.sched_priority = sched_get_priority_min(policy)+1;
	pthread_setschedparam( pthread_self(), policy, &param);

	int iterations = 0;
	for (;;)
	{
		pthread_barrier_wait(&noise_barrier);
		if(iterations <= 0)
		{	
			iterations = (int)(rand()%1000)+100; 			// draw how many iteration
			int pitch_noise_tmp = (int)(rand()%2000)-1000;	// draw extra angle caused by the wind
			int roll_noise_tmp = (int)(rand()%2000)-1000;

			pthread_mutex_lock(&mutex_Noise);
			pitch_noise = pitch_noise_tmp;
			roll_noise = roll_noise_tmp;
			pthread_mutex_unlock(&mutex_Noise);
		}
		iterations--;

	}

}