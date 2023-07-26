#include "periodic.h"

extern PID Pitch_PID;
extern PID Roll_PID;
extern PID Yaw_PID;
extern short throttle;
extern PID_terms Pitch_terms;
extern PID_terms Roll_terms;
extern PID_terms Yaw_terms;
/* Mutex for global variables */
extern pthread_mutex_t mutex_pitch;
extern pthread_mutex_t mutex_roll;
extern pthread_mutex_t mutex_yaw;
extern pthread_mutex_t mutex_throttle;

/* Shared variables */
struct sh_mem_control *sh_mem_control_ptr;
sem_t *mutex_sem, *spool_sem;
int fd_shm;

void *tPidThreadFunc(void *);   // PID functions thread

int thread_init();

timer_t	timerVar;               // Timer varibale
unsigned int_counter;           // Interrupt counter

/* Param for threat functions */
pthread_t tPidThread;
pthread_attr_t aPidThreadAttr;


int init_periodic()
{
	/* PID terms init*/
    PID_init(&Pitch_PID);
    set_PID_param(&Pitch_terms,0.5,0.1,0.1);
    set_PID_param(&Roll_terms,0.6,0.1,0.1);
    set_PID_param(&Yaw_terms,0.8,0.01,0.1);

    /* Shared variables for sending control- U(t) to Model*/
    sh_mem_control_init_writer(&sh_mem_control_ptr, &mutex_sem, &spool_sem, &fd_shm);

    /* Timer settings- periodic thread */
    struct itimerspec timerSpecStruct;
    timer_t	timerVar;
    struct sigevent timerEvent;

    /* Initialize threads attributes structures for FIFO scheduling */
	pthread_attr_init(&aPidThreadAttr);
	pthread_attr_setschedpolicy(&aPidThreadAttr, SCHED_FIFO);
	
	/* Initialize event to send signal SIGRTMAX */
	timerEvent.sigev_notify = SIGEV_THREAD;
    timerEvent.sigev_notify_function = tPidThreadFunc;
	timerEvent.sigev_notify_attributes = &aPidThreadAttr;

	int status;
	/* Create timer */
  	if ((status = timer_create(CLOCK_REALTIME, &timerEvent, &timerVar))) {
  		fprintf(stderr, "Error creating timer : %d\n", status);
  		return 0;
  	}

  	/* Set up timer structure with time parameters */
	timerSpecStruct.it_value.tv_sec = 1;
	timerSpecStruct.it_value.tv_nsec = 0;
	timerSpecStruct.it_interval.tv_sec = 0;
	timerSpecStruct.it_interval.tv_nsec = 500000;

	/* Change timer parameters and run */
  	timer_settime( timerVar, 0, &timerSpecStruct, NULL);

    return 0;
}

/* threads for 3 PID reg- thread function executing periodically*/
void *tPidThreadFunc(void *cookie) {

	int i = 0;

	int policy;/* Scheduling policy: FIFO or RR */
	struct sched_param param;/* Structure of other thread parameters */

	pthread_getschedparam( pthread_self(), &policy, &param); /* Read modify and set new thread priority */
	param.sched_priority = sched_get_priority_max(policy);
	pthread_setschedparam( pthread_self(), policy, &param);

    short error;
    double result_pitch = 0, result_roll = 0, result_yaw = 0;
	short tmp_throttle = 0;
	pthread_mutex_lock(&mutex_pitch);
	//printf("%d ", Pitch_PID.set_point);
	error = Pitch_PID.set_point - Pitch_PID.value_measured;
	Pitch_PID.integral = Pitch_PID.integral + error;
	result_pitch = Pitch_terms.kp * error + Pitch_terms.kd*(error - Pitch_PID.last_error) + Pitch_terms.ki * Pitch_PID.integral;
	Pitch_PID.last_error = error;
	pthread_mutex_unlock(&mutex_pitch);
		
	pthread_mutex_lock(&mutex_roll);
	//printf("%d ", Roll_PID.set_point);
	error = Roll_PID.set_point - Roll_PID.value_measured;
	Roll_PID.integral = Roll_PID.integral + error;
	result_roll = Roll_terms.kp * error + Roll_terms.kd*(error - Roll_PID.last_error) + Roll_terms.ki * Roll_PID.integral;
	Roll_PID.last_error = error;
	pthread_mutex_unlock(&mutex_roll);

	pthread_mutex_lock(&mutex_yaw);
	//printf("%d\n", Yaw_PID.set_point);
	error = Yaw_PID.set_point - Yaw_PID.value_measured;
	Yaw_PID.integral = Yaw_PID.integral + error;
	result_yaw = Yaw_terms.kp * error + Yaw_terms.kd*(error - Yaw_PID.last_error) + Yaw_terms.ki * Yaw_PID.integral;
	Yaw_PID.last_error = error;
	pthread_mutex_unlock(&mutex_yaw);

	pthread_mutex_unlock(&mutex_throttle);
	tmp_throttle = throttle;
	pthread_mutex_unlock(&mutex_throttle);

    /* Send to shared memory */
    sem_wait(mutex_sem);
    sh_mem_control_ptr->yaw_U = result_yaw;
	sh_mem_control_ptr->pitch_U = result_pitch;
	sh_mem_control_ptr->roll_U = result_roll;
	sh_mem_control_ptr->throttle = tmp_throttle;
    sem_post (mutex_sem);
    sem_post(spool_sem); 

	// Forced SensorsThread do the measurement
	pthread_barrier_wait(&measurement_barrier);

	return 0;
}
