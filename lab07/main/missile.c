#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>

#include "lcd.h" // coord_t
#include "tank.h"
#include "missile.h"
#include "config.h"
#include "hw.h"
#include "lcd.h"
#include "cursor.h"
#include "sound.h"
#include "pin.h"

#define FIFTH 5
#define HALF 2
#define SQUARED 2
#define THIRD 3
#define DOUBLE 2

// The same missile structure is used for all missiles in the game.
// All state variables for a missile are contained in the missile structure.
// There are no global variables maintained in the missile.c file.
// Each missile function requires a pointer argument to a missile struct.


/******************** Helper Functions ********************/
//TODO move these to missile.h 
//this function returns a random x value within lcd screen values   
int32_t get_random_x_position(){
    int32_t random_x_position = rand()%LCD_W; // calculate a random x position within board size  
    return random_x_position;
}

//returns a random y value within lcd screen values 
int32_t get_random_y_position(){
    int32_t random_y_position = rand()%LCD_H; // calculate a random x position within board size  
    return random_y_position;
}

//this function returns the total length of the coordinates passed in 
float get_total_length(float x_origin, float y_origin, float x_dest, float y_dest) {
    float delta_x = x_dest - x_origin;
    float delta_y = y_dest - y_origin;
    
    // Calculate the total length
    float length = sqrtf(powf(delta_x, SQUARED) + powf(delta_y, SQUARED));
    return length;
}



/******************** Missile Init Functions ********************/

// Different _init_ functions are used depending on the missile type.

// Initialize the missile as an idle missile. If initialized to the idle
// state, a missile doesn't appear nor does it move.
void missile_init_idle(missile_t *missile){
    missile->currentState = MISSILE_STATE_IDLE;
}


// Initialize the missile as a player missile. This function takes an (x, y)
// destination of the missile (as specified by the user). The origin is the current tank location
void missile_init_player(missile_t *missile, coord_t x_current, coord_t y_current, coord_t x_dest, coord_t y_dest){
    
    missile->type = MISSILE_TYPE_PLAYER;

    missile->x_origin = x_current;
    missile->y_origin = y_current;
    missile->x_dest = x_dest;
    missile->y_dest = y_dest;
    
    missile->currentState = MISSILE_STATE_INIT; 
}


// Initialize the missile as an enemy missile. This will randomly choose the
// origin and destination of the missile. The origin is somewhere near the
// top of the screen, and the destination is the very bottom of the screen.
void missile_init_enemy(missile_t *missile){
    // missile->type = MISSILE_TYPE_ENEMY;
    // // Set x,y origin to random place near the top of the screen (top quarter? – you choose!)
    // missile->x_origin = get_random_x_position(); //fires missile from random x position 
	// missile->y_origin = get_random_y_position()/FIFTH;
    //  // Set x,y destination to random location along the bottom of the screen
    // missile->x_dest = get_random_x_position();
    // missile->y_dest = LCD_H-1;
    // // Set current state
    // missile->currentState = MISSILE_STATE_INIT;
}


/******************** Missile Control & Tick Functions ********************/

// Used to indicate that a moving missile should be detonated. This occurs
// when an enemy missile is located within an explosion zone.
void missile_explode(missile_t *missile){
    missile->currentState = MISSILE_STATE_EXP_GROWING;
    missile->explode_me = true;
}


// Tick the state machine for a single missile.
void missile_tick(missile_t *missile){
    
    // /******************** SM TRANSISTIONS ********************/
    switch (missile->currentState) {
        //INIT state sets initial values 
        case MISSILE_STATE_INIT:
            // printf("MISSILE STATE INIT\n");
            missile->length = 0; 
            missile->explode_me = false;
            missile->total_length = get_total_length(missile->x_origin, missile->y_origin, missile->x_dest, missile->y_dest);
            
            missile->x_current = missile->x_origin;
            missile->y_current = missile->y_origin; 

            missile->radius = 1;
            
            missile->currentState = MISSILE_STATE_MOVING;
            break;

        //move missile until it needs to explode or impacts
        case MISSILE_STATE_MOVING:
            // printf("MISSILE STATE MOVING\n");
            //if collision is detected, move to explode grow 
            if (missile->explode_me){
                missile->currentState = MISSILE_STATE_EXP_GROWING;
            }
            //if missile hits the ground, transition to impacted
            else if (missile->length >= missile->total_length){
                //if its an enemy missile, transition to impacted
                // if(missile->type == MISSILE_TYPE_ENEMY){
                //     missile->currentState = MISSILE_STATE_IMPACTED;
                // }
                //if its a player missile, transition to explode
                if (missile->type == MISSILE_TYPE_PLAYER){
                     missile->currentState = MISSILE_STATE_EXP_GROWING;
                }
            }
            break;

        // grows missile explosion 
        case MISSILE_STATE_EXP_GROWING:
            //if explosion reaches max, transiton to exp shrink 
            if (missile->radius >= CONFIG_EXPLOSION_MAX_RADIUS){
                missile->currentState = MISSILE_STATE_EXP_SHRINKING;
            }
            break;

        //shrinks missile explosion 
        case MISSILE_STATE_EXP_SHRINKING:
            //if explosion is gone, transition to idle 
            if (missile->radius <= 0){
                missile->currentState = MISSILE_STATE_IDLE;
            }
            break;

        //state for if a missile is impacted 
        case MISSILE_STATE_IMPACTED:
            missile->currentState = MISSILE_STATE_IDLE;
            break;

        //state for an idle missile 
        case MISSILE_STATE_IDLE:
            break;
    }


    // /******************** SM ACTIONS ********************/
    
    switch (missile->currentState) {
        
        //missile init state 
        case MISSILE_STATE_INIT:
           break;

        //actions for moving the missile 
        case MISSILE_STATE_MOVING: //////
            //update length based on distance (pixels) traveled per tick 
            float fraction = missile->length / missile->total_length;
            // printf("Missile length: %f\n", missile->length);
            // printf("Missile total length: %f\n", missile->total_length);
            // printf("Fraction: %f\n\n",fraction);
            // printf("x origin: %ld\n", missile->x_origin);
            // printf("y origin: %ld\n", missile->y_origin);
            // printf("x dest: %ld\n", missile->x_dest);
            // printf("y dest: %ld\n", missile->y_dest);
            if (isnan(fraction) || isinf(fraction)) {
                // printf("Invalid fraction value: %f\n", fraction);
                return;
            }
            missile->x_current = missile->x_origin + fraction * (missile->x_dest - missile->x_origin);
            missile->y_current = missile->y_origin + fraction * (missile->y_dest - missile->y_origin);
            
            // //move the missile if the missile is an enemy missile
            // // if (missile->type == MISSILE_TYPE_ENEMY){
            // //     missile->length += CONFIG_ENEMY_MISSILE_DISTANCE_PER_TICK;
            // //     lcd_drawLine(missile->x_origin, missile->y_origin, missile->x_current, missile->y_current, CONFIG_COLOR_ENEMY_MISSILE);
            // // }

            //move the missile if the missile is a player missile
            if (missile->type == MISSILE_TYPE_PLAYER){
                missile->length += CONFIG_PLAYER_MISSILE_DISTANCE_PER_TICK;
                lcd_drawLine(missile->x_origin, missile->y_origin, missile->x_current, missile->y_current, CONFIG_COLOR_PLAYER_MISSILE);
            }
            break;

        //actions for growing the missile 
        case MISSILE_STATE_EXP_GROWING:
            missile->radius += CONFIG_EXPLOSION_RADIUS_CHANGE_PER_TICK;
            //if its a player missile, draw a green circle
            if (missile->type == MISSILE_TYPE_PLAYER){
                lcd_fillCircle(missile->x_current, missile->y_current, missile->radius, CONFIG_COLOR_PLAYER_MISSILE);  
            }
            //if its an enemy missile, draw a red circle 
            else if (missile->type == MISSILE_TYPE_ENEMY){
                lcd_fillCircle(missile->x_current, missile->y_current, missile->radius, CONFIG_COLOR_ENEMY_MISSILE);  
            }
            break;

        //actions for shrkinking the missile 
        case MISSILE_STATE_EXP_SHRINKING:
            missile->radius -= CONFIG_EXPLOSION_RADIUS_CHANGE_PER_TICK;
            
            //draw eplayer explosion shrinking
            if (missile->type == MISSILE_TYPE_PLAYER){
                lcd_fillCircle(missile->x_current, missile->y_current, missile->radius, CONFIG_COLOR_PLAYER_MISSILE);
            }
            //draw enemy exposion shrinking
            else if (missile->type == MISSILE_TYPE_ENEMY){
                lcd_fillCircle(missile->x_current, missile->y_current, missile->radius, CONFIG_COLOR_ENEMY_MISSILE);
            }
            break;

        //actions for impacted missiles (no actions needed)
        case MISSILE_STATE_IMPACTED:
            break;

        //actions for idle missiles 
        case MISSILE_STATE_IDLE:
            //basically defuse the missile
            missile->length = 0;
            missile->radius = 0;
            missile->explode_me=false; 
            break;
    }
}

/******************** Missile Status Functions ********************/

// Return the current missile position through the pointers *x,*y.
void missile_get_pos(missile_t *missile, coord_t *x, coord_t *y){
    *x = missile->x_current;
    *y = missile->y_current;
}


// Return the missile type.
missile_type_t missile_get_type(missile_t *missile){
    return missile->type;
}


// Return whether the given missile is moving.
bool missile_is_moving(missile_t *missile){
    return missile->currentState == MISSILE_STATE_MOVING;
}


// Return whether the given missile is exploding. If this missile
// is exploding, it can explode another intersecting missile.
bool missile_is_exploding(missile_t *missile){
    return (missile->currentState == MISSILE_STATE_EXP_GROWING) || (missile->currentState == MISSILE_STATE_EXP_SHRINKING);
}


// Return whether the given missile is idle.
bool missile_is_idle(missile_t *missile){
    return (missile->currentState == MISSILE_STATE_IDLE);
}


// Return whether the given missile is impacted.
bool missile_is_impacted(missile_t *missile){
    return (missile->currentState == MISSILE_STATE_IMPACTED);
}


// Return whether an object (missile) at the specified
// (x,y) position is colliding with the given missile. For a collision
// to occur, the missile needs to be exploding and the specified
// position needs to be within the explosion radius.
bool missile_is_colliding(missile_t *missile, coord_t x, coord_t y){
    //this function returns if a point is within the radius
    
    //checks to see if missile current positions are less than the radius away from the coordinates x and y
    int16_t delta_x = x - missile->x_current;
    int16_t delta_y = y - missile->y_current;  

    float radius_of_coordinates = (powf(delta_x,SQUARED)+powf(delta_y,SQUARED));
    bool within_radius = (radius_of_coordinates <= powf(missile->radius,SQUARED));//y^2 + x^2 <= radius^2
    
    //if position is in reach of missile and if the missile is exploding return true 
    if (within_radius && missile_is_exploding(missile)){ 
        return true;
    }
    //return false if its not within the radius or if its not exploding 
    else{
        return false;
    }
}