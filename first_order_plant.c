#include "first_order_plant.h"

/* Parameters values */
_plant_params plant_params = {0.05, 0.01, 0.01};

/* Input and outputs vars*/
double plant_output;

double calculate_plant(double plant_input, double prev_output) {

	double temp_input;
	double k1;

	/* Save output for a next step */
	prev_output = plant_output;

	/* Calculate output with Euler method */
	k1 = diff_eq(prev_output, plant_input);
	plant_output = prev_output + plant_params.ts * k1;
	
	// limites for outputs
	if(plant_output >MAX_ANGLE_VALUE)
		plant_output = MAX_ANGLE_VALUE;
	else if(plant_output <-MAX_ANGLE_VALUE)
		plant_output = -MAX_ANGLE_VALUE;

	return plant_output; // plant_output

}

double diff_eq(double prev_output, double input) {

	return -(1/plant_params.T) * prev_output + (plant_params.k/plant_params.T) * input;
}

double calculate_hight(double acceleration, double velocity, double cycle_time) {
	double d_hight = velocity*cycle_time + acceleration*(cycle_time*cycle_time)/2;
	return d_hight;
}

double calculate_velocity(double acceleration, double cycle_time)
{
	double d_velocity = acceleration*cycle_time;
	return d_velocity;
}