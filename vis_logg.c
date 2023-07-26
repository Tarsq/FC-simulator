#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "vis_logg.h"
#include "Gimbals_position.h"
#include "model.h"

/*** Visuallization part ***/
pthread_t tVisualThread;
pthread_attr_t aVisualThread;

/* critical sections */
extern ModelState Model_state;
extern pthread_rwlock_t lock_Modelstate;
extern short pitch_noise, roll_noise;
extern pthread_mutex_t mutex_Noise;

pthread_barrier_t visual_barrier;
pthread_barrier_t log_barrier;

/*** Logger part ***/
mqd_t outputMQueue; /* Mqueue variable */
struct	mq_attr outputMQueueAttr; /* Mqueue attributes structure */

pthread_t tLoggerThread; /* Thread variable */
pthread_attr_t aLoggerThreadAttr; /* Thread attributes structure */

int init_Visual(void){
    int status;
    pthread_attr_init(&aVisualThread);
	pthread_attr_setschedpolicy(&aVisualThread, SCHED_FIFO);

	pthread_barrier_init(&visual_barrier, NULL, 2); /* Initialize barrier */
    /* Create PIDs thread */
    if (status = pthread_create(&tVisualThread, &aVisualThread, tVisualThreadFunc, NULL))
    {
        fprintf(stderr, "Cannot create thread.\n");
        return 0;
    }
}

void *tVisualThreadFunc(void *cookie){

    int policy;/* Scheduling policy: FIFO or RR */
	struct sched_param param;/* Structure of other thread parameters */

	pthread_getschedparam( pthread_self(), &policy, &param); /* Read modify and set new thread priority */
	param.sched_priority = sched_get_priority_min(policy);
	pthread_setschedparam( pthread_self(), policy, &param);

	char buff[MAXLEN]; 
	int n, addr_length = 255;
	struct sockaddr_in socket_addr; /* Socket address structure */
	memset(&socket_addr, 0, sizeof(socket_addr));/* Set socket address parameters */
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_port = htons(1102);
	socket_addr.sin_addr.s_addr = INADDR_ANY;

    for(;;)
    {   
        /* Create socket */
        int my_socket = socket(PF_INET, SOCK_DGRAM, 0);
		if (my_socket == -1) 
		{
            fprintf(stderr, "Cannot create socket\n");
            return 0;
        }

		pthread_barrier_wait(&visual_barrier);
        int16_t angle[7];

        //pthread_mutex_lock(&mutex_Angles);
		pthread_rwlock_rdlock(&lock_Modelstate);
        angle[0] = Model_state.pitch;
        angle[1] = Model_state.roll;
        angle[2] = Model_state.yaw;
		angle[5] = Model_state.height;
		angle[6] = Model_state.vertical_velocity;
        pthread_rwlock_unlock(&lock_Modelstate);
		//pthread_mutex_unlock(&mutex_Angles);

		pthread_mutex_lock(&mutex_Noise);
		angle[3] = pitch_noise;
		angle[4] = roll_noise;
		pthread_mutex_unlock(&mutex_Noise);
            
        /* Send a message to server */
        sendto(my_socket, angle, sizeof(angle), MSG_CONFIRM, (const struct sockaddr *) &socket_addr, sizeof(socket_addr)); 
        
        /* Read the reply from server */      
        //n = recvfrom(my_socket, (char *)buff, MAXLEN, MSG_WAITALL, (struct sockaddr *) &socket_addr, &addr_length); 
        close(my_socket); 
    }
}

int init_Logger(void){
    int status;
	
	/* Initialize attributes structure */
	pthread_attr_init(&aLoggerThreadAttr);

	/* Set Message Queue size */
	outputMQueueAttr.mq_maxmsg = 2048;
	outputMQueueAttr.mq_msgsize = sizeof(int);

	/* Create Message Queue */
	if ((outputMQueue = mq_open("/outputMQ", O_CREAT | O_RDWR, 0777, &outputMQueueAttr)) == -1) {
		fprintf(stderr, "Creation of the mqueue failed.\n");
		return 0;
	}

	/* Create logger thread */
	if ((status = pthread_create( &tLoggerThread, NULL, tLoggerThreadFunc, &aLoggerThreadAttr))) {
		fprintf(stderr, "Cannot create thread.\n");
		return 0;
	}

	return 0;
}

/**
 * Finalize logger
 */
int finalize_logger() {

	/* Close Message Queue */
	mq_close(outputMQueue);

	return 0;
}

void *tLoggerThreadFunc(void *cookie){
    /* Output file descriptor */
	FILE * output_file;
	ModelState i;

	/* Scheduling policy: FIFO or RR */
	int policy = SCHED_FIFO;
	/* Structure of other thread parameters */
	struct sched_param param;

	/* Read modify and set new thread priority */
	param.sched_priority = sched_get_priority_min(policy)+1; /////// ????????????
	pthread_setschedparam( pthread_self(), policy, &param);

	/* Set the fiel header */
	output_file = fopen("sim_result.txt", "w");
	fprintf(output_file, "Drone state:\npitch roll yaw height vertical_velocity\n");
	fclose(output_file);
	int n;

	for (;;) {
		/* Wait until something will appears in queue */
		n = mq_receive(outputMQueue, (char *)&i, sizeof(ModelState), NULL);

		/* Append file with new data */
		output_file = fopen("sim_result.txt", "a");
		fprintf(output_file, "%d %d %d %d %d\n", i.pitch, i.roll, i.yaw, i.height, i.vertical_velocity);
		fclose(output_file);
	}

	return 0;

}