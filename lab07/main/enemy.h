#ifndef ENEMY_H
#define ENEMY_H

#include "lcd.h" // For coord_t type
#include "tank.h"

// Define game states
typedef enum {
  ENEMY_TANK_STATE_INIT,
  ENEMY_TANK_STATE_MOVING,
  ENEMY_TANK_STATE_IDLE,
} enemy_tank_state_t;

//tank struct 
typedef struct {

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

} enemy_tank_t;

// Initialize the enemy tank in a random position
void enemy_tank_init(enemy_tank_t *enemy_tank);

// Handle the enemy tank logic
void enemy_tank_tick();

// Explode the enemy tank
void enemy_tank_explode();

void enemy_tank_get_pos(coord_t *x, coord_t *y);

#endif // ENEMY_H




















