#ifndef MODEL_H_
#define MODEL_H_

struct ModelState {
    short pitch;
    short roll;
    short yaw;
    short height;
    short vertical_velocity;
};

typedef struct ModelState ModelState;

#endif