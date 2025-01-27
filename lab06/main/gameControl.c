#include <stdio.h>
#include <stdlib.h> // rand

#include "hw.h"
#include "lcd.h"
#include "sound.h"
#include "cursor.h"
#include "pin.h"
#include "missile.h"
#include "plane.h"
#include "gameControl.h"
#include "config.h"

// sound support
#include "missileLaunch.h"

// M3: Define stats constants
#define TEXT1_X 5
#define TEXT1_Y 5 
#define TEXT2_X 170 
#define TEXT2_Y 5
#define TEXT_SIZE 50

//Sound defines 
#define SOUND_VOLUME MAX_VOL/4

// All missiles
missile_t missiles[CONFIG_MAX_TOTAL_MISSILES];

// Alias into missiles array
missile_t *enemy_missiles = missiles+0;
missile_t *player_missiles = missiles+CONFIG_MAX_ENEMY_MISSILES;
missile_t *plane_missile = missiles+CONFIG_MAX_ENEMY_MISSILES+
									CONFIG_MAX_PLAYER_MISSILES;

// M3: Declare stats variables
int16_t player_missile_count;
int16_t impacted_missile_count;


//helper function to draw stats
//draw stats to the screen 
//first stat: count of player missiles shot 
//second stat: count of impacted missiles 
void draw_stats(){
	coord_t x;
	coord_t y;
	char text[TEXT_SIZE];
	
	//first stat: count of player missiles shot 
	sprintf(text, "# Player missiles: %d", player_missile_count);
	x = TEXT1_X;
	y = TEXT1_Y;
	lcd_drawString(x, y, text, CONFIG_COLOR_STATUS);

	//second stat: count of impacted missiles 
	sprintf(text, "# Impacted missiles: %d", impacted_missile_count);
	x = TEXT2_X;
	y = TEXT2_Y;
	lcd_drawString(x, y, text, CONFIG_COLOR_STATUS);
}


// Initialize the game control logic.
// This function initializes all missiles, planes, stats, etc.
void gameControl_init(void)
{
	// Initialize enemy missiles
	for (uint32_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES; i++) //initialize enemy missiles
		missile_init_enemy(enemy_missiles+i);
	//initialize player missiles
	for (uint32_t i = 0; i < CONFIG_MAX_PLAYER_MISSILES; i++) //initialize player missiles
		missile_init_idle(player_missiles+i);
	//initialize plane missile 
	missile_init_idle(plane_missile);

	// M3: Initialize plane
	if (missile_is_idle(plane_missile)) {
		plane_init(plane_missile);
	}

	// M3: Initialize stats
	player_missile_count = 0; 
	impacted_missile_count = 0; 

	// M3: Set sound volume
	sound_init(MISSILELAUNCH_SAMPLE_RATE);
	sound_set_volume(SOUND_VOLUME);
}

// Update the game control logic.
// This function calls the missile & plane tick functions, reinitializes
// idle enemy missiles, handles button presses, fires player missiles,
// detects collisions, and updates statistics.
void gameControl_tick(void)
{
	// Tick missiles in one batch
	for (uint32_t i = 0; i < CONFIG_MAX_TOTAL_MISSILES; i++){
		missile_tick(missiles+i);
		//count impacted here? 
	}

	// Reinitialize idle enemy missiles
	for (uint32_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES; i++)
		//if enemy missile is idle, init another enemy missile 
		if (missile_is_idle(enemy_missiles+i))
			missile_init_enemy(enemy_missiles+i);

	// M2: Check for button press. If so, launch a free player missile.
	// This code will indicate when any button in HW_BTN_MASK is pressed and
	// also indicate when all buttons in the mask are released. 
	static bool pressed = false;
	coord_t x, y;
	uint64_t btns;
	btns = ~pin_get_in_reg() & HW_BTN_MASK;
	//this is for shooting player missiles
	if (!pressed && btns) {
		pressed = true; // button pressed
		cursor_get_pos(&x, &y);
		// Check to see if a player missile is idle and launch it to the target (x,y) position.
		for (uint32_t i = 0; i < CONFIG_MAX_PLAYER_MISSILES; i++) {
            //if a player missile isn't being used, get ready for launch off 
			if (missile_is_idle(player_missiles+i)) {
				missile_init_player(player_missiles+i, x, y);
				//play the sound here
				sound_start(missileLaunch, MISSILELAUNCH_SAMPLES, false);
				player_missile_count ++; 
                break;
            }
        }
	} 
	//all released 
	else if (pressed && !btns) {
		pressed = false; // all released
	}

	// M2: Check for moving non-player missile collision with an explosion.
	//go through each of the enemy missiles 
	for (uint32_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES; i++) {
		//we want to only check moving missiles
		if (missile_is_moving(enemy_missiles+i)){
			missile_get_pos(enemy_missiles+i,&x,&y);
			//checks current enemy missile against player missiles that are exploding 
			for (uint32_t j = 0; j < CONFIG_MAX_PLAYER_MISSILES; j++) {
				// Check if the missile is colliding with an explosion
				if (missile_is_colliding(player_missiles+j, x, y)) {
					missile_explode(enemy_missiles+i);
					continue;
				}
			}
			//checks current enemy missile against other enemy missiles that are exploding 
			for (uint32_t k = 0; k < CONFIG_MAX_ENEMY_MISSILES; k++) {
				// Check if the missile is colliding with an explosion
				if (missile_is_colliding(enemy_missiles+k, x, y)) {
					missile_explode(enemy_missiles+i);
					continue;
				}
			}
		}	
	}

	// M3: Count non-player impacted missiles
	for (uint32_t i = 0; i < CONFIG_MAX_TOTAL_MISSILES; i++){
		//check to make sure we're only incrementing enemy or plane missiles 
		if ( ((missiles+i)->type == MISSILE_TYPE_ENEMY) || ((missiles+i)->type == MISSILE_TYPE_PLANE) ){
			//increment impacted_missile_count if the missile is impacted 
			if (missile_is_impacted(missiles+i)){
				impacted_missile_count ++;
			}
		}
	}

	// M3: Tick plane & draw stats
	plane_tick();

	// M3: Check for flying plane collision with an explosion.
	if (plane_is_flying()){
		plane_get_pos(&x,&y);
		//checks plane against player missiles that are exploding 
		for (uint32_t j = 0; j < CONFIG_MAX_PLAYER_MISSILES; j++) {
			// Check if the plane is colliding with an explosion
			if (missile_is_colliding(player_missiles+j, x, y)) {
				plane_explode();
				continue;
			}
		}
		
		//checks plane against enemy missiles that are exploding 
		for (uint32_t k = 0; k < CONFIG_MAX_ENEMY_MISSILES; k++) {
			// Check if the plane is colliding with a missile explosion
			if (missile_is_colliding(enemy_missiles+k, x, y)) {
				plane_explode();
				continue;
			}
		}
	}

	//Check for plane missile collisions with explosions 
	if (missile_is_moving(plane_missile)){
		missile_get_pos(plane_missile,&x,&y);
		//check plane missile against player missiles 
		for (uint32_t j = 0; j < CONFIG_MAX_PLAYER_MISSILES; j++) {
			// Check if the plane missile is colliding with an explosion
			if (missile_is_colliding(player_missiles+j, x, y)) {
				missile_explode(plane_missile);
				continue;
			}
		}
		
		//checks plane missile against enemy missiles that are exploding 
		for (uint32_t k = 0; k < CONFIG_MAX_ENEMY_MISSILES; k++) {
			// Check if the plane is colliding with a missile explosion
			if (missile_is_colliding(enemy_missiles+k, x, y)) {
				missile_explode(plane_missile);
				continue;
			}
		}
	}
	
	//draw stats onto the screen 
	draw_stats();
}

