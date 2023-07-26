#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "periodic.h"
#include "Gimbals_position.h"

/* Global variable for PID regulators*/
PID Pitch_PID;
PID Roll_PID;
PID Yaw_PID;
short throttle;
PID_terms Pitch_terms;
PID_terms Roll_terms;
PID_terms Yaw_terms;
/* Mutex for global variables */
pthread_mutex_t mutex_pitch = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_roll = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_yaw = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_throttle = PTHREAD_MUTEX_INITIALIZER;

/* Signal handler function */
static void sig_handler(int);
/* PID for delegated tasks */
pid_t model_pid;
pid_t visualization_pid;
pid_t controller_pid;
int FC_init();

/*** Main function ***/
int main(int argc, char *argv[])
{
    int status;
   
    char * model_arg[3] = {"./model", NULL};
    model_pid = fork();

    if (model_pid == 0)
    { 
        /* This is fast child process, just run it */
 		execvp(model_arg[0], model_arg);
    }
    else
    {
        controller_pid = fork();
        if(controller_pid == 0)
        {
            execlp("python3", "python3", "../RC_controller.py", (char*)0);
        }
        else
        {
            visualization_pid = fork();
            if(visualization_pid == 0)
            {
                execlp("python3", "python3", "../visuallization2.py", (char*)0);
            }
            else
            {
                status = FC_init();
                return status;
            }

        }
    }

    return 0;
}

int FC_init()
{
    init_RC();      // server from Gimbals
    init_sensors(); // make measurement
    init_periodic();// periodic function
    /* Create empty signal set to run sigsuspend */
    sigset_t empty_mask;
    sigemptyset(&empty_mask);

    /* Prepare sigaction struct for handler */
    struct sigaction action;
    action.sa_handler = sig_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    /*  Register signal handler for SIGINT */
    if (sigaction(SIGINT, &action, NULL) < 0) {
        fprintf(stderr, "Cannot register SIGINT handler.\n");
        return -1;
    }
   
    while (1){
        sigsuspend(&empty_mask);
    }
    return 0;
}

/* Signal handler function */
void sig_handler(int sig) {
    if (sig == SIGINT) {
        fprintf(stderr, "Flight controller is quiting!\n");
        kill(model_pid, SIGINT);
        kill(visualization_pid, SIGINT);
        kill(controller_pid, SIGINT);
        exit(0);
    }
}