#ifndef MODEL_OUTPUT_H
#define MODEL_OUTPUT_H

#define MAXLEN 100
#include <pthread.h>
extern pthread_barrier_t noise_barrier;

int init_ModelOutputs(void);
void *tPlantOutputsThreadFunc(void *);

int init_WindDisruption(void);
void *tWindDisruptionThreadFunc(void *);

#endif