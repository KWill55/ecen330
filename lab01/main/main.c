#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "lcd.h"
#include "pac.h"

static const char *TAG = "lab01";

#define delayMS(ms) \
	vTaskDelay(((ms)+(portTICK_PERIOD_MS-1))/portTICK_PERIOD_MS)

//----------------------------------------------------------------------------//
// Car Implementation - Begin
//----------------------------------------------------------------------------//

// Car constants
#define CAR_CLR rgb565(220,30,0)
#define WINDOW_CLR rgb565(180,210,238)
#define TIRE_CLR BLACK
#define HUB_CLR GRAY

// TODO: Finish car part constants
#define CAR_W 60
#define CAR_H 32

#define BODY_X0 0
#define BODY_Y0 12 
#define BODY_X1 59 
#define BODY_Y1 24 

#define UPPER_BODY_X0 1 
#define UPPER_BODY_Y0 0 
#define UPPER_BODY_X1 39 
#define UPPER_BODY_Y1 11 

#define WINDOW_RADIUS 2
#define LEFT_WINDOW_X0 3
#define LEFT_WINDOW_Y0 1 
#define LEFT_WINDOW_X1 18 
#define LEFT_WINDOW_Y1 8 
#define RIGHT_WINDOW_X0 21 
#define RIGHT_WINDOW_Y0 1 
#define RIGHT_WINDOW_X1 37 
#define RIGHT_WINDOW_Y1 8 

#define LEFT_INNER_TIRE_XC 11
#define LEFT_INNER_TIRE_YC 24
#define LEFT_INNER_TIRE_R 4
#define LEFT_TIRE_XC 11 
#define LEFT_TIRE_YC 24
#define LEFT_TIRE_R 7
#define RIGHT_INNER_TIRE_XC 48 
#define RIGHT_INNER_TIRE_YC 24 
#define RIGHT_INNER_TIRE_R 4
#define RIGHT_TIRE_XC 48 
#define RIGHT_TIRE_YC 24 
#define RIGHT_TIRE_R 7

#define HOOD_X0 40 
#define HOOD_Y0 9 
#define HOOD_X1 40 
#define HOOD_Y1 11 
#define HOOD_X2 59 
#define HOOD_Y2 11 


/**
 * @brief Draw a car at the specified location.
 * @param x      Top left corner X coordinate.
 * @param y      Top left corner Y coordinate.
 * @details Draw the car components relative to the anchor point (top, left).
 */
void drawCar(coord_t x, coord_t y)
{ //draws a car onto the screen
	// Implement car procedurally with lcd geometric primitives.
	//body
	lcd_drawRect2(BODY_X0 + x, BODY_Y0 + y, BODY_X1 + x, BODY_Y1 + y, CAR_CLR);
	lcd_fillRect2(BODY_X0 + x, BODY_Y0 + y, BODY_X1 + x, BODY_Y1 + y, CAR_CLR);
	//upper body
	lcd_drawRect2(UPPER_BODY_X0 + x, UPPER_BODY_Y0 + y, UPPER_BODY_X1 + x, UPPER_BODY_Y1 + y, CAR_CLR);
	lcd_fillRect2(UPPER_BODY_X0 + x, UPPER_BODY_Y0 + y, UPPER_BODY_X1 + x, UPPER_BODY_Y1 + y, CAR_CLR);
	//left window
	lcd_drawRoundRect2(LEFT_WINDOW_X0 + x, LEFT_WINDOW_Y0 + y, LEFT_WINDOW_X1 + x, LEFT_WINDOW_Y1 + y, WINDOW_RADIUS, WINDOW_CLR);
	lcd_fillRoundRect2(LEFT_WINDOW_X0 + x, LEFT_WINDOW_Y0 + y, LEFT_WINDOW_X1 + x, LEFT_WINDOW_Y1 + y, WINDOW_RADIUS, WINDOW_CLR);
	//right window
	lcd_drawRoundRect2(RIGHT_WINDOW_X0 + x, RIGHT_WINDOW_Y0 + y, RIGHT_WINDOW_X1 + x, RIGHT_WINDOW_Y1 + y, WINDOW_RADIUS, WINDOW_CLR);
	lcd_fillRoundRect2(RIGHT_WINDOW_X0 + x, RIGHT_WINDOW_Y0 + y, RIGHT_WINDOW_X1 + x, RIGHT_WINDOW_Y1 + y, WINDOW_RADIUS, WINDOW_CLR);
	//left tire
	lcd_drawCircle(LEFT_TIRE_XC + x, LEFT_TIRE_YC + y, LEFT_TIRE_R, TIRE_CLR);
	lcd_fillCircle(LEFT_TIRE_XC + x, LEFT_TIRE_YC + y, LEFT_TIRE_R, TIRE_CLR);
	lcd_drawCircle(LEFT_INNER_TIRE_XC + x, LEFT_INNER_TIRE_YC + y, LEFT_INNER_TIRE_R, HUB_CLR);
	lcd_fillCircle(LEFT_INNER_TIRE_XC + x, LEFT_INNER_TIRE_YC + y, LEFT_INNER_TIRE_R, HUB_CLR);
	//right tire
	lcd_drawCircle(RIGHT_TIRE_XC + x, RIGHT_TIRE_YC + y, RIGHT_TIRE_R, TIRE_CLR);
	lcd_fillCircle(RIGHT_TIRE_XC + x, RIGHT_TIRE_YC + y, RIGHT_TIRE_R, TIRE_CLR);
	lcd_drawCircle(RIGHT_INNER_TIRE_XC + x, RIGHT_INNER_TIRE_YC + y, RIGHT_INNER_TIRE_R, HUB_CLR);
	lcd_fillCircle(RIGHT_INNER_TIRE_XC + x, RIGHT_INNER_TIRE_YC + y, RIGHT_INNER_TIRE_R, HUB_CLR);
	//hood
	lcd_drawTriangle(HOOD_X0 + x, HOOD_Y0 + y, HOOD_X1 + x, HOOD_Y1 + y, HOOD_X2 + x, HOOD_Y2 + y, CAR_CLR);
	lcd_fillTriangle(HOOD_X0 + x, HOOD_Y0 + y, HOOD_X1 + x, HOOD_Y1 + y, HOOD_X2 + x, HOOD_Y2 + y, CAR_CLR);
}

//----------------------------------------------------------------------------//
// Car Implementation - End
//----------------------------------------------------------------------------//

// Main display constants
#define BACKGROUND_CLR rgb565(0,60,90)
#define TITLE_CLR GREEN
#define STATUS_CLR WHITE
#define STR_BUF_LEN 12 // string buffer length
#define FONT_SIZE 2
#define FONT_W (LCD_CHAR_W*FONT_SIZE)
#define FONT_H (LCD_CHAR_H*FONT_SIZE)
#define STATUS_W (FONT_W*3)

#define WAIT 2000 // milliseconds
#define DELAY_EX3 20 // milliseconds

// Object position and movement
#define OBJ_X 100
#define OBJ_Y 100
#define OBJ_MOVE 3 // pixels
#define CAR_START_X -45
#define CAR_STOP_X 330
#define PACMAN_START_X -45
#define PACMAN_STOP_X 330


//Goes through 5 stages of animations moving across the screen
void app_main(void)
{
	ESP_LOGI(TAG, "Start up");
	lcd_init();
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_setFontSize(FONT_SIZE);
	lcd_drawString(0, 0, "Device is starting up...", TITLE_CLR);
	printf("This is printing to the terminal (terminal)\n");
	delayMS(WAIT);


// Exercise 1 - Draw car in one location.
	lcd_fillScreen(BACKGROUND_CLR);// * Fill screen with background color
	lcd_drawString(0, 0, "Exercise 1", TITLE_CLR); // * Draw string "Exercise 1" at top left of screen with title color
	drawCar(OBJ_X, OBJ_Y);// * Draw car at OBJ_X, OBJ_Y
	delayMS(2000);// * Wait 2 seconds

// Exercise 2 - Draw moving car (Method 1), one pass across display.
//Method 1: Clear the entire display and redraw all objects each iteration. Use a loop and increment x by OBJ_MOVE each iteration. Start x off screen (negative coordinate).
	for (coord_t x=CAR_START_X; x<=CAR_STOP_X; x+=OBJ_MOVE){ // Move loop:
		lcd_setFontSize(FONT_SIZE);
		lcd_fillScreen(BACKGROUND_CLR); // * Fill screen with background color
		lcd_drawString(0, 0, "Exercise 2", TITLE_CLR); // * Draw string "Exercise 2" at top left of screen with title color
		drawCar(x,OBJ_Y); // * Draw car at x, OBJ_Y

		char str[50];    
    	sprintf(str, "x position: %3ld", x);
    	lcd_drawString(0, LCD_H-FONT_H, str, STATUS_CLR); // * Display the x position of the car at bottom left of screen
	}


// Exercise 3 - Draw moving car (Method 2), one pass across display.
// Method 2: Move by erasing car at old position, then redrawing at new position. Objects that don't change or move are drawn once.
	lcd_setFontSize(FONT_SIZE);
	lcd_fillScreen(BACKGROUND_CLR); // * Fill screen once with background color
	lcd_drawString(0, 0, "Exercise 3", TITLE_CLR); // * Draw string "Exercise 3" at top left of screen with title color

	for (coord_t x=CAR_START_X; x<=CAR_STOP_X; x+=OBJ_MOVE){ // Move loop:
		//draw objects
		char str[50];    
    	sprintf(str, "x position: %3ld", x);
    	lcd_drawString(0, LCD_H-FONT_H, str, STATUS_CLR); // * Display the x position of the car at bottom left of screen
		drawCar(x,OBJ_Y); // * Draw car at new position
		
		//erase objects
		lcd_drawRect2(x, OBJ_Y, x+CAR_W, OBJ_Y+CAR_H, BACKGROUND_CLR); // * Erase car at old position by drawing a rectangle with background color
		lcd_fillRect2(x, OBJ_Y, x+CAR_W, OBJ_Y+CAR_H, BACKGROUND_CLR);
		lcd_drawRect2(0, LCD_H-FONT_H, LCD_W, LCD_H, BACKGROUND_CLR); // * Erase status at bottom by drawing a rectangle with background color
		lcd_fillRect2(0, LCD_H-FONT_H, LCD_W, LCD_H, BACKGROUND_CLR);

		//delay
		delayMS(20); // After running the above first, add a 20ms delay within the loop
	}


// Exercise 4 - Draw moving car (Method 3), one pass across display.
// Method 3:First, draw all objects into a cleared, off-screen frame buffer. Then, transfer the entire frame buffer to the screen.
	lcd_frameEnable(); // * Enable the frame buffer

	for (coord_t x=CAR_START_X; x<=CAR_STOP_X; x+=OBJ_MOVE){// Move loop:
		lcd_fillScreen(BACKGROUND_CLR); // * Fill screen (frame buffer) with background color
		lcd_drawString(0, 0, "Exercise 4", TITLE_CLR); // * Draw string "Exercise 4" at top left with title color
		drawCar(x,OBJ_Y); // * Draw car at x, OBJ_Y
		char str[50];    
    	sprintf(str, "x position: %3ld", x);
    	lcd_drawString(0, LCD_H-FONT_H, str, STATUS_CLR); // * Display the x position of the car at bottom left of screen
		lcd_writeFrame();// * Write the frame buffer to the LCD
	}

	lcd_frameDisable(); // deallocate the frame buffer

// Exercise 5 - Draw an animated Pac-Man moving across the display. Cycle through each sprite when moving the Pac-Man character.
	lcd_frameEnable(); // * Enable the frame buffer
	uint16_t i = 0;
	const uint8_t pidx[] = {0, 1, 2, 1};
	for(;;){ // Nest the move loop inside a forever loop:
		for (coord_t x=PACMAN_START_X; x<=PACMAN_STOP_X; x+=OBJ_MOVE){ // Move loop:
			lcd_fillScreen(BACKGROUND_CLR);// * Fill screen (frame buffer) with background color
			lcd_drawString(0, 0, "Exercise 5", TITLE_CLR); // * Draw string "Exercise 5" at top left with title color
			lcd_drawBitmap(x, OBJ_Y, pac[pidx[i++ % sizeof(pidx)]],PAC_W, PAC_H, YELLOW); // * Draw Pac-Man at x, OBJ_Y with yellow color; cycle through sprites
			char str[50];    
    		sprintf(str, "x position: %3ld", x);
    		lcd_drawString(0, LCD_H-FONT_H, str, STATUS_CLR); // * Display the x position of pac man at bottom left of screen
			lcd_writeFrame(); // * Write the frame buffer to the LCD
			}
	}
}
