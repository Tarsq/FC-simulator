#ifndef VIS_LOGG_H_
#define VIS_LOGG_H_

#include <mqueue.h>

/* Barriers */
extern pthread_barrier_t visual_barrier;
extern pthread_barrier_t log_barrier;
/* Mqueue variable */
extern mqd_t outputMQueue; 

int init_Visual(void);
void *tVisualThreadFunc(void *cookie);
int init_Logger(void);
void *tLoggerThreadFunc(void *cookie);
int finalize_logger(void);

#endif 