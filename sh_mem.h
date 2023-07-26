#ifndef SH_MEM_H_
#define SH_MEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>

struct sh_mem_control {
	int pitch_U;
	int roll_U;
	int yaw_U;
	int throttle;
};

int sh_mem_control_init_writer(struct sh_mem_control **shared_mem_ptr, sem_t **mutex_sem, sem_t** spool_sem, int *fd_shm_ptr);
int sh_mem_control_init_reader(struct sh_mem_control **shared_mem_ptr, sem_t **mutex_sem, sem_t** spool_sem, int *fd_shm_ptr);

#endif