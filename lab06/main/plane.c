#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>

#include "lcd.h" // coord_t
#include "missile.h"
#include "plane.h"
#include "config.h"
#include "hw.h"
#include "lcd.h"
#include "cursor.h"
#include "sound.h"
#include "pin.h"


#define SQUARED 2
#define HALF 2
#define PLANE_X_ORIGIN LCD_W
#define PLANE_Y_ORIGIN 75
#define PLANE_X_DEST 0 
#define PLANE_Y_DEST 75


// Define game states
typedef enum {
  PLANE_STATE_INIT,
  PLANE_STATE_MOVING,
  PLANE_STATE_IDLE,
} plane_state_t;


// Global plane variables
static plane_state_t current_state; // Tracks current state
bool explode_me = false; 
float length; 
float total_length;
coord_t x_current;
coord_t y_current; 
float time_offscreen = 0;
static missile_t *global_plane_missile; //a pointer to the plane missile in missile.c



/******************** Plane Init Function ********************/

// Initialize the plane state machine. Pass a pointer to the missile
// that will be (re)launched by the plane. It will only have one missile.
void plane_init(missile_t *missile){
    global_plane_missile = missile; 
    missile->explode_me = false;
    x_current = PLANE_X_ORIGIN;
    y_current = PLANE_Y_ORIGIN;
}

/******************** Plane Control & Tick Functions ********************/

// Trigger the plane to explode.
void plane_explode(void){
    explode_me = true;
}

//function that gets the total distance the plane must travel 
float get_total_plane_length(float x_origin, float y_origin, float x_dest, float y_dest) {
    float delta_x = x_dest - x_origin;
    float delta_y = y_dest - y_origin;
    
    // Calculate the total length
    float length = sqrtf(powf(delta_x, SQUARED) + powf(delta_y, SQUARED));
    return length;
}

//use the values to find vertices for the plane 
void draw_plane(coord_t x,coord_t y){
    //point 1: left vertice || point 2: top right vertice || point 3: bottom left vertice 
    lcd_fillTriangle(x, y, x+CONFIG_PLANE_WIDTH, y+(CONFIG_PLANE_HEIGHT/HALF), x+CONFIG_PLANE_WIDTH, y-(CONFIG_PLANE_HEIGHT/HALF), CONFIG_COLOR_PLANE);
}

//this function returns a random x value within lcd screen values 
int32_t get_random_x_position2(){
    int32_t random_x_position = rand()%LCD_W; // calculate a random x position within board size  
    return random_x_position;
}

// State machine tick function.
void plane_tick(void){
    /******************** SM TRANSISTIONS ********************/
    switch (current_state) {
        //transition to move state 
        case PLANE_STATE_INIT:
            total_length = get_total_plane_length(PLANE_X_ORIGIN, PLANE_Y_ORIGIN, PLANE_X_DEST, PLANE_Y_DEST);
            explode_me = false; 
            x_current = PLANE_X_ORIGIN;
            y_current = PLANE_Y_ORIGIN;
            length = 0;
            time_offscreen = 0;
            
            current_state = PLANE_STATE_MOVING;
            break;

        //if plane is hit with explosion or makes it across screen transition to idle
        case PLANE_STATE_MOVING:
            //if the plane is hit by an explosion, transition to idle 
            if (explode_me == true){
                current_state = PLANE_STATE_IDLE;
            }

            //if plane reaches the other side of the screen, transistion to idle 
            if (length >= total_length){
                current_state = PLANE_STATE_IDLE;
            }
            break;

        //after 4 seconds transition to init state 
        case PLANE_STATE_IDLE:
            //if plane has not been offscreen for the required time 
            if (time_offscreen < CONFIG_PLANE_IDLE_TIME_TICKS){
                current_state = PLANE_STATE_IDLE;
            }
            //if plane has been offscreen for the required time 
            else{
                current_state = PLANE_STATE_INIT;
            }
            time_offscreen ++;
            break; 
    }

    /******************** SM ACTIONS ********************/
    switch (current_state) {
        //Initialize plane 
        case PLANE_STATE_INIT:
            //i initilaized the plane in transition
            break;

        //Move plane from right to left || fire missile at a certain point 
        case PLANE_STATE_MOVING:
            //shoot missile when plane distance reaches half way across the screen 
            int16_t x = get_random_x_position2();
            //if the x_current is at the random point specified fire the plane missile
            if ((x_current <= x+1) && (x_current >= x-1)){
                //if the plane missile is idle and not on the screen, launch the plane missile
                if (missile_is_idle(global_plane_missile)) {
                    missile_init_plane(global_plane_missile, x_current, y_current);
                    global_plane_missile->currentState = MISSILE_STATE_INIT; 
                }
            }

            //update position and draw plane 
            float fraction = length / total_length;
            x_current = PLANE_X_ORIGIN + fraction * (PLANE_X_DEST - PLANE_X_ORIGIN);
            length += CONFIG_PLANE_DISTANCE_PER_TICK;
            draw_plane(x_current,y_current);
            break;

        //no action required 
        case PLANE_STATE_IDLE:
            break; 

    }  
}

/******************** Plane Status Function ********************/

// Return the current plane position through the pointers *x,*y.
void plane_get_pos(coord_t *x, coord_t *y){
    *x = x_current;
    *y = y_current;
}

// Return whether the plane is flying.
bool plane_is_flying(void){
    //if the plane is moving return true
    if (current_state == PLANE_STATE_MOVING){
        return true; 
    }
    //if the plane is not moving return false 
    else{
        return false; 
    }
}
