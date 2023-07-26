#ifndef FIRST_ORDER_PLANT_
#define FIRST_ORDER_PLANT_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

#define MAX_ANGLE_VALUE 18000

/* Parameters struct */
struct _plant_params {
	double k;
	double T;
	double ts;
};

typedef struct _plant_params _plant_params;

double calculate_plant(double plant_input, double prev_output) ;
double diff_eq(double prev_output, double input);
double calculate_hight(double acceleration, double velocity, double cycle_time);
double calculate_velocity(double acceleration, double cycle_time);

#endif