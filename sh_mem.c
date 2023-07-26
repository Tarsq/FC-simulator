#include "sh_mem.h"

int sh_mem_control_init_writer(struct sh_mem_control **shared_mem_ptr, sem_t **mutex_sem, sem_t** spool_sem, int * fd_shm_ptr)
{
    if ((*mutex_sem = sem_open("/sem-mutex", O_CREAT, 0660, 0)) == SEM_FAILED) {
        fprintf(stderr, "Cannot create semaphore.\n");
		return 0;
    }

    // Create spool semaphore
    if ((*spool_sem = sem_open("/sem-spool", O_CREAT, 0660, 0)) == SEM_FAILED) {
        fprintf(stderr, "Cannot create semaphore.\n");
		return 0;
    }
    
    // Get shared memory 
    if ((*fd_shm_ptr = shm_open("/my-shared-mem", O_RDWR | O_CREAT, 0660)) == -1) {
        fprintf(stderr, "Cannot create shared memory.\n");
		return 0;
    }

    // Map shared memory
    if ((*shared_mem_ptr = mmap(NULL, sizeof (struct sh_mem_control), PROT_READ | PROT_WRITE, MAP_SHARED, *fd_shm_ptr, 0)) == MAP_FAILED) {
        fprintf(stderr, "Cannot map shared memory.\n");
		return 0;
    }
}

int sh_mem_control_init_reader(struct sh_mem_control **shared_mem_ptr, sem_t **mutex_sem, sem_t** spool_sem, int * fd_shm_ptr)
{
    if ((*mutex_sem = sem_open("/sem-mutex", O_CREAT, 0660, 0)) == SEM_FAILED) {
        fprintf(stderr, "Cannot create semaphore.\n");
		return 0;
    }

    //  Create spool semaphore
    if ((*spool_sem = sem_open("/sem-spool", O_CREAT, 0660, 0)) == SEM_FAILED) {
        fprintf(stderr, "Cannot create semaphore.\n");
		return 0;
    }
    
    // Get shared memory 
    if ((*fd_shm_ptr = shm_open("/my-shared-mem", O_RDWR | O_CREAT, 0660)) == -1) {
        fprintf(stderr, "Cannot create shared memory.\n");
		return 0;
    }

    //  Truncate share memory
    if (ftruncate(*fd_shm_ptr, sizeof (struct sh_mem_control)) == -1) {
        fprintf(stderr, "Cannot truncate shared memory.\n");
		return 0;
    }

    // Map shared memory
    if ((*shared_mem_ptr = mmap(NULL, sizeof (struct sh_mem_control), PROT_READ | PROT_WRITE, MAP_SHARED, *fd_shm_ptr, 0)) == MAP_FAILED) {
        fprintf(stderr, "Cannot map shared memory.\n");
		return 0;
    }
}