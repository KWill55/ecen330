
//game intro with a bunch of exposions
//victory after 5 successful shots 
//graves where the old tanks died 

#include <stdio.h>
#include <stdlib.h> // rand

#include "hw.h"
#include "lcd.h"
#include "sound.h"
#include "cursor.h"
#include "pin.h"
#include "missile.h" 
#include "tank.h"
#include "enemy.h"
#include "gameControl.h"
#include "config.h"

// sound support
// #include "missileLaunch.h"

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
// missile_t *enemy_missiles = missiles+0;
// missile_t *player_missiles = missiles+CONFIG_MAX_ENEMY_MISSILES;

// M3: Declare stats variables
int16_t player_missile_count = 0;
int16_t destroyed_enemies_count = 0;

//global tank
tank_t tank;
enemy_tank_t enemy_tank;
missile_t player_missile;


//helper function to draw stats
//draw stats to the screen 
//first stat: count of player missiles shot 
//second stat: count of impacted missiles 
void draw_stats(){
	coord_t x;
	coord_t y;
	char text[TEXT_SIZE];
	
	//first stat: count of player missiles shot 
	sprintf(text, "# Missiles Shot: %d", player_missile_count);
	x = TEXT1_X;
	y = TEXT1_Y;
	lcd_drawString(x, y, text, CONFIG_COLOR_STATUS);

	//second stat: count of impacted missiles 
	sprintf(text, "# Destroyed Enemies: %d", destroyed_enemies_count);
	x = TEXT2_X;
	y = TEXT2_Y;
	lcd_drawString(x, y, text, CONFIG_COLOR_STATUS);
}


// Initialize the game control logic.
// This function initializes all missiles, stats, etc.
void gameControl_init(void)
{

	// Initialize enemy missiles
	// for (uint32_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES; i++) //initialize enemy missiles
	// 	missile_init_enemy(enemy_missiles+i);
	// //initialize player missiles
	// for (uint32_t i = 0; i < CONFIG_MAX_PLAYER_MISSILES; i++) //initialize player missiles
	// 	missile_init_idle(player_missiles+i);

	// M3: Initialize tank
    lcd_setFontSize(3);
		char win_text[20]; // Allocate enough space for the string
		sprintf(win_text, "GO!!");
		lcd_drawString(LCD_W / 4, LCD_H / 2, win_text, PURPLE);
		lcd_setFontSize(1);
	
	tank_init(&tank, &player_missile);
	enemy_tank_init(&enemy_tank); //&enemy_tank
	missile_init_idle(&player_missile);






	// M3: Initialize stats
	// player_missile_count = 0; 
	// impacted_missile_count = 0; 

	// M3: Set sound volume
	// sound_init(MISSILELAUNCH_SAMPLE_RATE);
	// sound_set_volume(SOUND_VOLUME);
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

	// // Reinitialize idle enemy missiles
	// for (uint32_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES; i++)
	// 	//if enemy missile is idle, init another enemy missile 
	// 	if (missile_is_idle(enemy_missiles+i))
	// 		missile_init_enemy(enemy_missiles+i);

	// M2: Check for button press. If so, launch a free player missile.
	// This code will indicate when any button in HW_BTN_MASK is pressed and
	// also indicate when all buttons in the mask are released. 
	static bool pressed = false;
	uint64_t btns;
	btns = ~pin_get_in_reg() & HW_BTN_MASK;
	//this is for shooting player missiles
	
	if (!pressed && btns) {
		pressed = true; // Button is pressed

		// Check if the player missile is idle and initialize it
		if (missile_is_idle(&player_missile)) {
			missile_init_player(&player_missile, tank.x_current,tank.y_current,tank.x_destination,tank.y_destination); // Initialize the missile
			player_missile_count ++;
			// Play the missile launch sound (uncomment when ready)
			// sound_start(missileLaunch, MISSILELAUNCH_SAMPLES, false);
			// printf("Player missile launched to target x: %d, y: %d\n", x, y);
		}

	} 	
	else if (pressed && !btns) {
    	pressed = false; // Button is released
	}
	
	//tick the different game pieces 
	
	if (destroyed_enemies_count < 5){
	tank_tick();
	enemy_tank_tick();
		missile_tick(&player_missile);
	}
	else{
		lcd_setFontSize(3);
		char win_text[20]; // Allocate enough space for the string
		sprintf(win_text, "YOU WIN!!");
		lcd_drawString(LCD_W / 4, LCD_H / 2, win_text, PURPLE);
		lcd_setFontSize(1);
		// gameControl_init();

	}

	coord_t mx,my; //missile coordinate
	coord_t ex,ey; //enemy coordinate

	//checks plane against player missiles that are exploding 
	if (missile_is_moving(&player_missile)){
		// printf("Missile is moving\n");
		missile_get_pos(&player_missile,&mx,&my);
		// printf("Missile Position: x = %ld, y = %ld\n", mx, my);
		enemy_tank_get_pos(&ex,&ey);
		// printf("Enemy Tank Position: x = %ld, y = %ld\n", ex, ey);
		
		if (missile_is_moving(&player_missile)) {
    		missile_get_pos(&player_missile, &mx, &my);
    		enemy_tank_get_pos(&ex, &ey);

			// Check if missile is within a 20-unit range of the tank
			if (abs(mx - ex) <= 30 && abs(my - ey) <= 30) {
				// printf("Collision detected: Missile exploded\n");
				missile_explode(&player_missile);
				enemy_tank_explode();
				destroyed_enemies_count ++;
    		}
		}
	}
		 
	draw_stats();
}

