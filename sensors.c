#include "sensors.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>


pthread_t tSensorsThread;
pthread_attr_t aSensorsThreadAttr;

pthread_barrier_t measurement_barrier;

int init_sensors()
{
    int status;
    pthread_attr_init(&aSensorsThreadAttr);
    pthread_attr_setschedpolicy(&aSensorsThreadAttr, SCHED_FIFO);

    /* Initialize barrier */
	pthread_barrier_init(&measurement_barrier, NULL, 2);

    if(status = pthread_create(&tSensorsThread, &aSensorsThreadAttr,tSensorsThreadFunc, NULL))
    {
        fprintf(stderr, "Cannot create sensors thread.\n ");
        return 1;
    }
}

void *tSensorsThreadFunc(void *cookie)
{
    //printf("Hello from sensors thread\n");
    int policy;
    struct sched_param param;
    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = sched_get_priority_max(policy)-1;
    pthread_setschedparam(pthread_self(), policy, &param);

    /* Variable for sensors server */
    short buff[3];
    int n, addr_length = 255;

	/* Socket address structure */
	struct sockaddr_in socket_addr;
    //struct sockaddr client_addr;

    /* Initialize socket address to 0*/
    memset(&socket_addr, 0, sizeof(socket_addr));
    /* Set socket address parameters */
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(1110);
    socket_addr.sin_addr.s_addr = INADDR_ANY;

    for(;;)
    {
        /* Create socket */
        int my_socket = socket(PF_INET, SOCK_DGRAM, 0);

        if (my_socket == -1) {
            fprintf(stderr, "Cannot create socket\n");
            return 0;
        }

        char start[2]; // order to make measurement
        start[0] = 1;

        pthread_barrier_wait(&measurement_barrier);

        /* Send a message to server */
        sendto(my_socket, start, sizeof(start), MSG_CONFIRM, (const struct sockaddr *) &socket_addr, sizeof(socket_addr)); 
        /* Read the reply from server */      
        n = recvfrom(my_socket, (char *)buff, MAXLEN, MSG_WAITALL, (struct sockaddr *) &socket_addr, &addr_length); 

        pthread_mutex_lock(&mutex_pitch);
        Pitch_PID.value_measured = buff[0];
        pthread_mutex_unlock(&mutex_pitch);

        pthread_mutex_lock(&mutex_roll);
        Roll_PID.value_measured = buff[1];
        pthread_mutex_unlock(&mutex_roll);

        pthread_mutex_lock(&mutex_yaw);
        Yaw_PID.value_measured = buff[2];
        pthread_mutex_unlock(&mutex_yaw);
        close(my_socket); 
    }	
}