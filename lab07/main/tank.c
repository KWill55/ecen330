#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>

#include "lcd.h" // coord_t
#include "missile.h"
#include "tank.h"
#include "config.h"
#include "hw.h"
#include "lcd.h"
#include "cursor.h"
#include "sound.h"
#include "pin.h"

#define TANK_X_ORIGIN 25//LCD_W/2
#define TANK_Y_ORIGIN 25//LCD_H/2

#define SQUARED 2
#define HALF 2

//Global Variables
float time_offscreen = 0;
// int32_t player_missile_count = 0;

//Global Variable 
static missile_t *g_missile;
static tank_t *g_tank;

/******************** Helper Functions ********************/

//function that gets the total distance the tank must travel 
float get_total_tank_length(float x_origin, float y_origin, float x_dest, float y_dest) {
    float delta_x = x_dest - x_origin;
    float delta_y = y_dest - y_origin;
    
    // Calculate the total length
    float length = sqrtf(powf(delta_x, SQUARED) + powf(delta_y, SQUARED));
    return length;
}

//this function returns a random x value within lcd screen values 
int32_t get_random_x_position2(){
    int32_t random_x_position = rand()%LCD_W; // calculate a random x position within board size  
    return random_x_position;
}

/******************** Tank Init Function ********************/

// Initialize the tank state machine. Pass a pointer to the missile
// that will be (re)launched by the tank. It will only have one missile.
void tank_init(tank_t *tank, missile_t *missile){
    g_missile = missile;
    g_tank = tank; 
    
    g_missile->explode_me = false;
    g_tank->x_current = TANK_X_ORIGIN;
    g_tank->y_current = TANK_Y_ORIGIN;
}

/******************** Tank Control & Tick Functions ********************/

// Trigger the tank to explode.
void tank_explode(){
    g_tank->explode_me = true;
}

//draw the tank using the values x and y 
//x and y are the current position of the end of the tank's barrel
//parts of the tank: body, gun, wheels
void draw_tank(coord_t x, coord_t y) {

    switch (g_tank->direction){
        case UP:
            // Draw tank body
            lcd_fillRect(
                x - (CONFIG_TANK_BODY_WIDTH / 2), 
                y, 
                CONFIG_TANK_BODY_WIDTH, 
                CONFIG_TANK_BODY_HEIGHT, 
                CONFIG_COLOR_TANK
            );

            // Draw tank gun
            lcd_fillRect(
                x - (CONFIG_TANK_GUN_WIDTH / 2), 
                y - CONFIG_TANK_GUN_HEIGHT, 
                CONFIG_TANK_GUN_WIDTH, 
                CONFIG_TANK_GUN_HEIGHT, 
                CONFIG_COLOR_TANK
            );

            // Draw right track
            lcd_fillRect2(
                x + (CONFIG_TANK_BODY_WIDTH / 2), 
                y, 
                x + (CONFIG_TANK_BODY_WIDTH / 2) + CONFIG_TANK_WHEELS_WIDTH, 
                y + CONFIG_TANK_BODY_HEIGHT, 
                CONFIG_COLOR_WHEELS
            );

            // Draw left track
            lcd_fillRect2(
                x - (CONFIG_TANK_BODY_WIDTH / 2), 
                y, 
                x - (CONFIG_TANK_BODY_WIDTH / 2) - CONFIG_TANK_WHEELS_WIDTH, 
                y + CONFIG_TANK_BODY_HEIGHT, 
                CONFIG_COLOR_WHEELS
            );
            break;

        case DOWN:
            lcd_fillRect(
                x - (CONFIG_TANK_BODY_WIDTH / 2), 
                y - CONFIG_TANK_BODY_HEIGHT, 
                CONFIG_TANK_BODY_WIDTH, 
                CONFIG_TANK_BODY_HEIGHT, 
                CONFIG_COLOR_TANK
            );

            lcd_fillRect(
                x - (CONFIG_TANK_GUN_WIDTH / 2), 
                y, 
                CONFIG_TANK_GUN_WIDTH, 
                CONFIG_TANK_GUN_HEIGHT, 
                CONFIG_COLOR_TANK
            );

            lcd_fillRect2(
                x + (CONFIG_TANK_BODY_WIDTH / 2), 
                y - CONFIG_TANK_BODY_HEIGHT, 
                x + (CONFIG_TANK_BODY_WIDTH / 2) + CONFIG_TANK_WHEELS_WIDTH, 
                y, 
                CONFIG_COLOR_WHEELS
            );

            lcd_fillRect2(
                x - (CONFIG_TANK_BODY_WIDTH / 2), 
                y - CONFIG_TANK_BODY_HEIGHT, 
                x - (CONFIG_TANK_BODY_WIDTH / 2) - CONFIG_TANK_WHEELS_WIDTH, 
                y, 
                CONFIG_COLOR_WHEELS
            );
            break;


        case LEFT:
            lcd_fillRect(
                x, 
                y - (CONFIG_TANK_BODY_HEIGHT / 2), 
                CONFIG_TANK_BODY_HEIGHT, 
                CONFIG_TANK_BODY_WIDTH, 
                CONFIG_COLOR_TANK
            );

            lcd_fillRect(
                x - CONFIG_TANK_GUN_HEIGHT, 
                y - (CONFIG_TANK_GUN_WIDTH / 2), 
                CONFIG_TANK_GUN_HEIGHT, 
                CONFIG_TANK_GUN_WIDTH, 
                CONFIG_COLOR_TANK
            );

            lcd_fillRect2(
                x, 
                y + (CONFIG_TANK_BODY_HEIGHT / 2), 
                x + CONFIG_TANK_BODY_WIDTH, 
                y + (CONFIG_TANK_BODY_HEIGHT / 2) + CONFIG_TANK_WHEELS_WIDTH, 
                CONFIG_COLOR_WHEELS
            );

            lcd_fillRect2(
                x, 
                y - (CONFIG_TANK_BODY_HEIGHT / 2), 
                x + CONFIG_TANK_BODY_WIDTH, 
                y - (CONFIG_TANK_BODY_HEIGHT / 2) - CONFIG_TANK_WHEELS_WIDTH, 
                CONFIG_COLOR_WHEELS
            );
            break;

        case RIGHT:
            lcd_fillRect(
                x, 
                y - (CONFIG_TANK_BODY_HEIGHT / 2), 
                CONFIG_TANK_BODY_WIDTH, 
                CONFIG_TANK_BODY_HEIGHT, 
                CONFIG_COLOR_TANK
            );

            lcd_fillRect(
                x + CONFIG_TANK_BODY_WIDTH, 
                y - (CONFIG_TANK_GUN_WIDTH / 2), 
                CONFIG_TANK_GUN_HEIGHT, 
                CONFIG_TANK_GUN_WIDTH, 
                CONFIG_COLOR_TANK
            );

            lcd_fillRect2(
                x, 
                y + (CONFIG_TANK_BODY_HEIGHT / 2), 
                x + CONFIG_TANK_BODY_WIDTH, 
                y + (CONFIG_TANK_BODY_HEIGHT / 2) + CONFIG_TANK_WHEELS_WIDTH, 
                CONFIG_COLOR_WHEELS
            );

            lcd_fillRect2(
                x, 
                y - (CONFIG_TANK_BODY_HEIGHT / 2), 
                x + CONFIG_TANK_BODY_WIDTH, 
                y - (CONFIG_TANK_BODY_HEIGHT / 2) - CONFIG_TANK_WHEELS_WIDTH, 
                CONFIG_COLOR_WHEELS
            );
            break;

    }
}



// State machine tick function.
void tank_tick(){
    
    
    
    /******************** SM TRANSISTIONS ********************/
    switch (g_tank->current_state) {
        //transition to move state 
        case TANK_STATE_INIT:
            g_tank->explode_me = false; 
            g_tank->x_current = TANK_X_ORIGIN;
            g_tank->y_current = TANK_Y_ORIGIN;
    //         time_offscreen = 0;
            
            g_tank->current_state = TANK_STATE_MOVING;
            break;

        //if tank is hit with explosion transition to idle
        case TANK_STATE_MOVING:
            //if the tank is hit by an explosion, transition to idle 
            if (g_tank->explode_me == true){
                g_tank->current_state = TANK_STATE_IDLE;
            }

    //     //after 2 seconds transition to init state 
    //     case TANK_STATE_IDLE:
    //         //if plane has not been offscreen for the required time 
    //         if (time_offscreen < CONFIG_TANK_IDLE_TIME_TICKS){ 
    //             tank->current_state = TANK_STATE_IDLE;
    //         }
    //         //if plane has been offscreen for the required time 
    //         else{
    //             tank->current_state = TANK_STATE_INIT;
    //         }
    //         time_offscreen ++;
    //         break; 
    }

    // /******************** SM ACTIONS ********************/
    switch (g_tank->current_state) {
        //Initialize tank 
        case TANK_STATE_INIT:
            draw_tank(g_tank->x_current, g_tank->y_current);
            g_tank->direction = UP;
            // g_tank->x_current = TANK_X_ORIGIN;
            // g_tank->y_current = TANK_Y_ORIGIN;
            // g_tank->y_destination = TANK_X_ORIGIN; 
            // g_tank->x_destination = TANK_Y_ORIGIN;

            //i initilaized the tank in transition switch statement 
            
            break;

        //determine direction and move tank 
        case TANK_STATE_MOVING:
            coord_t cx, cy;
            cursor_get_pos(&cx, &cy);
            // printf("%ld\n",cx); //see where the cursor currently is 

            //determine direcion the tank will go 
            if (cx <= 0+5){ //if cursor on left of screen 
                g_tank->direction = LEFT;
                g_tank->x_current -= CONFIG_TANK_DISTANCE_PER_TICK;
                g_tank->x_destination = 0; 
                g_tank->y_destination = g_tank->y_current;
            }
            else if (cx >= LCD_W-5){ //if cursor on left of screen 
                g_tank->direction = RIGHT;
                g_tank->x_current += CONFIG_TANK_DISTANCE_PER_TICK;
                g_tank->x_destination = LCD_W; 
                g_tank->y_destination = g_tank->y_current;
            }
            else if (cy <= 0+5){ //if cursor on left of screen 
                g_tank->direction = UP;
                g_tank->y_current -= CONFIG_TANK_DISTANCE_PER_TICK;
                g_tank->y_destination = 0; 
                g_tank->x_destination = g_tank->x_current;
            }
            else if (cy >= LCD_H-5){ //if cursor on left of screen 
                g_tank->direction = DOWN;
                g_tank->y_current += CONFIG_TANK_DISTANCE_PER_TICK;
                g_tank->y_destination = LCD_H; 
                g_tank->x_destination = g_tank->x_current;
            }


            //if tank hits the end of the screen, bring them to the other side
            if (g_tank->x_current < 0){
                g_tank->x_current = LCD_W;
            }
            else if (g_tank->x_current > LCD_W){
                g_tank->x_current = 0;
            }
            else if (g_tank->y_current > LCD_H){
                g_tank->y_current = 0;
            }
            else if (g_tank->y_current < 0){
                g_tank->y_current = LCD_H;
            }



            //draw the updated tank location
            draw_tank(g_tank->x_current, g_tank->y_current);


            //variables and bit mask to detect button press
            static bool pressed = false;
	        uint64_t btns;
	        btns = ~pin_get_in_reg() & HW_BTN_MASK;

            //if button is pressed 
            if (!pressed && btns) {
                pressed = true; // button pressed
                
                    if (missile_is_idle(g_missile)) { //TODO & or no 
                    missile_init_player(g_missile, g_tank->x_current, g_tank->y_current, g_tank->x_destination, g_tank->y_destination);
                    g_missile->currentState = MISSILE_STATE_INIT; 
                    // sound_start(missileLaunch, MISSILELAUNCH_SAMPLES, false);
                    // player_missile_count ++;
                    }
                
                // printf("missile launch\n");
                // cursor_get_pos(&tank->x_current, &tank->y_current); //maybe change this
                // Check to see if a player missile is idle and launch it to the target (x,y) position.
            }    
            //all released 
            else if (pressed && !btns) {
                pressed = false; // all released
            }

            break;

        // no action required 
        case TANK_STATE_IDLE:
            break; 

    }  
}

/******************** Plane Status Function ********************/

// Return the current plane position through the pointers *x,*y.
void tank_get_pos(coord_t *x, coord_t *y){
    *x = g_tank->x_current;
    *y = g_tank->y_current;
}

// Return whether the plane is flying.
bool tank_is_moving(){
    //if the plane is moving return true
    if (g_tank->current_state == TANK_STATE_MOVING){
        return true; 
    }
    //if the plane is not moving return false 
    else{
        return false; 
    }
}
