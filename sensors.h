#ifndef SENSORS_H
#define SENSORS_H
 
#include <pthread.h>
#include "pid.h"

#define MAXLEN 100 
extern PID Pitch_PID;
extern PID Roll_PID;
extern PID Yaw_PID;
/* Mutex for global variables */
extern pthread_mutex_t mutex_pitch;
extern pthread_mutex_t mutex_roll;
extern pthread_mutex_t mutex_yaw;

/* Sensors barrier */
extern pthread_barrier_t measurement_barrier;

void *tSensorsThreadFunc(void *);
int init_sensors();

#endif /* SENSORS_H */