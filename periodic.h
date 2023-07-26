#ifndef PERIODIC_H_
#define PERIODIC_H_

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <semaphore.h>

#include "pid.h"
#include "sh_mem.h"
#include "sensors.h"

#define MAXLEN 100 
int init_periodic();

#endif
