#ifndef TANK_H_
#define TANK_H_

#include "missile.h"

//tank direction enum
typedef enum {
    LEFT,//TODO set these equal to the values that will be right of screen, bottom, etc 
    RIGHT,
    UP,
    DOWN
} tank_direction_t;

// Define game states
typedef enum {
  TANK_STATE_INIT,
  TANK_STATE_MOVING,
  TANK_STATE_IDLE,
} tank_state_t;

//tank struct 
typedef struct {

	// Missile type (player, enemy, enemy plane).
	tank_direction_t direction;

	// Current state. The 'enum' is defined in tank.c file.
	int32_t current_state; //TODO should this be tank_state_t

	// Tracks the current x,y of missile.
	coord_t x_current;
	coord_t y_current;

	// A flag to indicate the missile should be detonated when moving.
	bool explode_me;

	// While exploding, this tracks the current radius.
	float radius;

	coord_t x_destination;
    coord_t y_destination;

} tank_t;


/******************** Plane Init Function ********************/

// Initialize the tank state machine. Pass a pointer to the missile
// that will be (re)launched by the tank. It will only have one missile.
void tank_init(tank_t *tank, missile_t *missile);

/******************** Plane Control & Tick Functions ********************/

// Trigger the plane to explode.
void tank_explode(void);

// State machine tick function.
void tank_tick(void); 

/******************** Plane Status Function ********************/

// Return the current plane position through the pointers *x,*y.
void tank_get_pos(coord_t *x, coord_t *y); //i added plane_t

// Return whether the plane is flying.
bool tank_is_moving(void); //i added plane_t

#endif // TANK_H_
