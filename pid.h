#ifndef PID_H_
#define PID_H_

struct PID {
	short set_point;
	short value_measured;
	int last_error,integral;
	int result;
};

struct PID_terms{
	double kp,kd,ki;
};

typedef struct PID PID;
typedef struct PID_terms PID_terms;

void PID_init(PID *pid_struct);
void set_PID_param(PID_terms * pid_term_struct,double kp, double kd, double ki);
void PID_alghoritm(PID *pid_struct, PID_terms *pid_terms_struct , int *PID_sp, int *PID_measured);

#endif