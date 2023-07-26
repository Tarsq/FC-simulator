#ifndef GIMBALS_POSITION_H_
#define GIMBALS_POSITION_H_
#include <pthread.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "pid.h"

/* Param for gimbal position receiver */
#define MAXLEN 100 

extern PID Pitch_PID;
extern PID Roll_PID;
extern PID Yaw_PID;
extern short throttle;
/* Mutex for global variables */
extern pthread_mutex_t mutex_pitch;
extern pthread_mutex_t mutex_roll;
extern pthread_mutex_t mutex_yaw;
extern pthread_mutex_t mutex_throttle;

/* Function creates screen thread */
int init_RC();


#endif /*GIMBALS_POSITION_H_ */