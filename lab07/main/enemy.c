
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "lcd.h" // coord_t
#include "enemy.h"
#include "config.h"

#define TANK_WIDTH CONFIG_TANK_BODY_WIDTH
#define TANK_HEIGHT CONFIG_TANK_BODY_HEIGHT

// Global Variables
static coord_t enemy_x;
static coord_t enemy_y;

static enemy_tank_t *g_enemy_tank;

float enemy_time_offscreen = 0;

/******************** Helper Functions ********************/

// Generate a random x position within screen bounds
coord_t get_random_x_position3() {
    return rand() % (LCD_W - TANK_WIDTH) + TANK_WIDTH / 2;
}

// Generate a random y position within screen bounds
coord_t get_random_y_position3() {
    return rand() % (LCD_H - TANK_HEIGHT) + TANK_HEIGHT / 2;
}

/******************** Enemy Tank Functions ********************/

// Draw the enemy tank at its current position
void enemy_draw_tank(coord_t x, coord_t y) {
    // Draw tank body
    lcd_fillRect(
        x - (CONFIG_TANK_BODY_WIDTH / 2), 
        y, 
        CONFIG_TANK_BODY_WIDTH, 
        CONFIG_TANK_BODY_HEIGHT, 
        RED
    );

    // Draw tank gun
    lcd_fillRect(
        x - (CONFIG_TANK_GUN_WIDTH / 2), 
        y - CONFIG_TANK_GUN_HEIGHT, 
        CONFIG_TANK_GUN_WIDTH, 
        CONFIG_TANK_GUN_HEIGHT, 
        RED
    );

    // Draw right track
    lcd_fillRect2(
        x + (CONFIG_TANK_BODY_WIDTH / 2), 
        y, 
        x + (CONFIG_TANK_BODY_WIDTH / 2) + CONFIG_TANK_WHEELS_WIDTH, 
        y + CONFIG_TANK_BODY_HEIGHT, 
        BLACK
    );

    // Draw left track
    lcd_fillRect2(
        x - (CONFIG_TANK_BODY_WIDTH / 2), 
        y, 
        x - (CONFIG_TANK_BODY_WIDTH / 2) - CONFIG_TANK_WHEELS_WIDTH, 
        y + CONFIG_TANK_BODY_HEIGHT, 
        BLACK
    );
}



// Trigger the tank to explode.
void enemy_tank_explode(){
    g_enemy_tank->explode_me = true;
}

// Initialize the enemy tank in a random position
void enemy_tank_init(enemy_tank_t *enemy_tank) {
    //set position of enemy tank 
    g_enemy_tank = enemy_tank;

    g_enemy_tank->explode_me = false;
    enemy_draw_tank(g_enemy_tank->x_current, g_enemy_tank->y_current);
}


// Handle the enemy tank logic
void enemy_tank_tick(){
    
    enemy_draw_tank(g_enemy_tank->x_current, g_enemy_tank->y_current);
 /******************** SM TRANSISTIONS ********************/
    switch (g_enemy_tank->current_state) {
        //transition to move state 
        case ENEMY_TANK_STATE_INIT:
            g_enemy_tank->explode_me = false; 
            enemy_x = get_random_x_position3();
            enemy_y = get_random_y_position3();
            g_enemy_tank->x_current = enemy_x;
            g_enemy_tank->y_current = LCD_H; //enemy_y;

            g_enemy_tank->current_state = ENEMY_TANK_STATE_MOVING;
            break;

        //if tank is hit with explosion transition to idle
        case ENEMY_TANK_STATE_MOVING:
            enemy_draw_tank(g_enemy_tank->x_current, g_enemy_tank->y_current);
            if (g_enemy_tank->explode_me == true){
                g_enemy_tank->current_state = ENEMY_TANK_STATE_IDLE;
                // lcd_fillRect2(g_enemy_tank->x_current-5, g_enemy_tank->y_current-10, g_enemy_tank->x_current+5, g_enemy_tank->y_current+10, GRAY);
            }

            break;

        case ENEMY_TANK_STATE_IDLE:
            //if plane has not been offscreen for the required time 
            if (enemy_time_offscreen < CONFIG_TANK_IDLE_TIME_TICKS){ 
                g_enemy_tank->current_state = TANK_STATE_IDLE;
            }
            //if plane has been offscreen for the required time 
            else{
                g_enemy_tank->current_state = TANK_STATE_INIT;
            }
            enemy_time_offscreen ++;
            break; 
    }

    // /******************** SM ACTIONS ********************/
    switch (g_enemy_tank->current_state) {
        //Initialize tank 
        case ENEMY_TANK_STATE_INIT:
            enemy_draw_tank(g_enemy_tank->x_current, g_enemy_tank->y_current);
            break;

        //determine direction and move tank 
        case ENEMY_TANK_STATE_MOVING:
            
             if (g_enemy_tank->y_current < 0){
                g_enemy_tank->y_current = LCD_H;
                enemy_x = get_random_x_position3();
                g_enemy_tank->x_current = enemy_x;
            }
            
            g_enemy_tank->y_current-=2; 
            enemy_draw_tank(g_enemy_tank->x_current, g_enemy_tank->y_current);
            break;

        // no action required 
        case ENEMY_TANK_STATE_IDLE:
            break; 
    }
}

// Return the current plane position through the pointers *x,*y.
void enemy_tank_get_pos(coord_t *x, coord_t *y){
    *x = g_enemy_tank->x_current;
    *y = g_enemy_tank->y_current;
}












































