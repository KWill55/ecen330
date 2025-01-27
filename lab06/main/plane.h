#ifndef PLANE_H_
#define PLANE_H_

//TODO can i get rid of this include? i'm not supposed to have this in a .h
#include "missile.h"

/******************** Plane Init Function ********************/

// Initialize the plane state machine. Pass a pointer to the missile
// that will be (re)launched by the plane. It will only have one missile.
void plane_init(missile_t *plane_missile);

/******************** Plane Control & Tick Functions ********************/

// Trigger the plane to explode.
void plane_explode(void);

// State machine tick function.
void plane_tick(void); //i changed it from void 

/******************** Plane Status Function ********************/

// Return the current plane position through the pointers *x,*y.
void plane_get_pos(coord_t *x, coord_t *y); //i added plane_t

// Return whether the plane is flying.
bool plane_is_flying(void); //i added plane_t

#endif // PLANE_H_
