#include <stdio.h>
#include <stdlib.h>
#include "pid.h"

void set_PID_param(PID_terms * pid_term_struct,double kp, double kd, double ki)
{
	pid_term_struct->kp = kp;
	pid_term_struct->kd = kd;
	pid_term_struct->ki = ki;	
}

void PID_init(PID *pid_struct)
{
	pid_struct->set_point = 0;
	pid_struct->value_measured = 0;
	pid_struct->integral = 0;
	pid_struct->last_error = 0;
}

void PID_alghoritm(PID * pid_struct, PID_terms *pid_terms_struct , int *PID_sp, int *PID_measured)
{
	int error = *PID_sp - *PID_measured;
	pid_struct->integral = pid_struct->integral + error;
	pid_struct->result = pid_terms_struct->kp * error + pid_terms_struct->kd * (error - pid_struct->last_error) + pid_terms_struct->ki * pid_struct->integral;
	pid_struct->last_error = error;
}

 