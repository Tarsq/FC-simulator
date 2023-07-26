#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <mqueue.h>
#include <math.h>

#include "model.h"
#include "Gimbals_position.h"
#include "model_output.h"
#include "sh_mem.h"
#include "first_order_plant.h"
#include "vis_logg.h" 

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define MAXLEN 100
#define DRONE_MASS 2 // kg
#define G 9.81
#define MAX_DRON_ACC G*3 // m/s^2
#define PI 3.141592654

ModelState Model_state;
double position, velocity, acceleration;
short pitch_noise, roll_noise; 
pthread_mutex_t mutex_Noise = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t lock_Modelstate;

static void exit_handler(int);

int main(void)
{	
	/* Receiving signal SIGINT */
    sigset_t empty_mask;
    sigemptyset(&empty_mask);

    struct sigaction exit_action;
    exit_action.sa_handler = exit_handler;
    sigemptyset(&exit_action.sa_mask);
    exit_action.sa_flags = 0;

    if (sigaction(SIGINT, &exit_action, NULL) < 0) {
        fprintf(stderr, "Cannot register SIGINT handler.\n");
        return 0;
    }

	pthread_rwlock_init(&lock_Modelstate, NULL);

	init_Visual();
	init_Logger();
	init_ModelOutputs();
	init_WindDisruption();
	

    int status;
    struct sh_mem_control *sh_mem_control_ptr;
    sem_t *mutex_sem, *spool_sem;
    int fd_shm;

    sh_mem_control_init_reader(&sh_mem_control_ptr, &mutex_sem, &spool_sem, &fd_shm);
    sem_post(mutex_sem);

    int tmp_pitch_U = 0,tmp_roll_U = 0,tmp_yaw_U = 0;   		// przychodzace sterowania
	short tmp_throttle = 0;
	double tmp_pitch_out = 0,tmp_roll_out = 0 ,tmp_yaw_out=0;	// wyliczone wyjscia
	short tmp_pitch_noise = 0, tmp_roll_noise =0;				// wyliczone zaklucenia

	int i = 0;
	int j = 0;
    while(1)
    {
        sem_wait(spool_sem);
        sem_wait(mutex_sem);
        tmp_pitch_U = sh_mem_control_ptr->pitch_U;
        tmp_roll_U = sh_mem_control_ptr->roll_U;
        tmp_yaw_U = sh_mem_control_ptr->yaw_U;
		tmp_throttle = sh_mem_control_ptr -> throttle;
		sem_post(mutex_sem);

		double k1,k2,k;
		if(tmp_yaw_out > -9000 && tmp_yaw_out < 9000)
		{
			k =  tmp_yaw_out/9000; // /90 degree
			k1 = 1-k;
			k2 = k;
		}
		else
		{
			k =  (tmp_yaw_out-9000)/9000; // /90 degree
			k1 = k;
			k2 = 1-k;
		}
		tmp_pitch_out = tmp_pitch_out*k1 + tmp_roll_out*k2;
		tmp_roll_out = tmp_roll_out*k1 + tmp_pitch_out*k2;

		tmp_pitch_out = calculate_plant(tmp_pitch_U,tmp_pitch_out);
		tmp_roll_out = calculate_plant(tmp_roll_U,tmp_roll_out);

		acceleration = (tmp_throttle+18000) * MAX_DRON_ACC/36000* cos(tmp_pitch_out/100*PI/180) * cos(tmp_roll_out/100*PI/180)-G;
	
		tmp_yaw_out = calculate_plant(tmp_yaw_U,tmp_yaw_out);
		velocity += calculate_velocity(acceleration, 5.0/10000);
		position += calculate_hight(acceleration, velocity, 5.0/10000);
		if(position < 0)
		{
			position = 0;
			velocity = 0;
			acceleration = 0;
		}

		/* ADD NOISE made by wind */
		pthread_mutex_lock(&mutex_Noise);
		tmp_pitch_noise = pitch_noise;
		tmp_roll_noise = roll_noise;
		pthread_mutex_unlock(&mutex_Noise);

		/* WRITE OUTPUTS to global variables */
		pthread_rwlock_wrlock(&lock_Modelstate);
	    Model_state.pitch = tmp_pitch_out + tmp_pitch_noise;
        Model_state.roll = tmp_roll_out + tmp_roll_noise;
        Model_state.yaw = tmp_yaw_out;
		Model_state.height = position;
		Model_state.vertical_velocity = velocity;
		pthread_rwlock_unlock(&lock_Modelstate);

		j++;
		if(!(j%50))
		{
			j = 0;
			pthread_barrier_wait(&noise_barrier); //update noise
		}

		i++;
		if (!(i%500)) {
			i = 0;
			pthread_barrier_wait(&visual_barrier); // Visual update
		
			/* Send the plant output to the logger */
			pthread_rwlock_rdlock(&lock_Modelstate);
			mq_send(outputMQueue, (const char *)&Model_state, sizeof(Model_state), 0);
			pthread_rwlock_unlock(&lock_Modelstate);
		}
    }
}

/* Exit signal handler function */
void exit_handler(int sig) {
	finalize_logger();
    fprintf(stderr, "Model is quiting!\n");
    exit(0);
}
