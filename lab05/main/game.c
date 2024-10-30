#include "game.h"
#include "graphics.h"
#include "board.h"
#include "nav.h"
#include "hw.h"
#include "lcd.h"
#include "pin.h"
#include "config.h"
#include "com.c"
#include <stdio.h>

// Define game states
typedef enum {
  INIT_ST,
  NEW_GAME_ST,
  WAIT_MARK_ST,
  MARK_ST,
  WAIT_RESTART_ST
} game_state_t;

// Game variables
static game_state_t current_state; // Tracks current state
static char current_player; // Tracks current player (X or O)
static bool game_over;    // Tracks if game is over
static int8_t row, column;  // Navigator position
static uint8_t incoming_byte; // To hold incoming data from communication


#define MESSAGE_SIZE 50
#define NUM_MASK 0x07
#define NUM_BIT_SHIFT 4

// Initialize game components
void game_init(void) {
  current_state = INIT_ST; // Start at initial state
  game_over = false;
}

// Function to switch whose turn it is
void switch_player() { //if x, then o; if not x, then x
    current_player = (current_player == 'X') ? 'O' : 'X';
}

// State Machine tick function to update game logic
void game_tick(void){
  
  //switch for State Machine Transisitons 
  switch (current_state) {
    case INIT_ST:
      // Transition to the NEW_GAME_ST
      current_state = NEW_GAME_ST;
      break;

    case NEW_GAME_ST:
      current_state = WAIT_MARK_ST; // Move to wait state
      break;

    case WAIT_MARK_ST:
      if (!pin_get_level(HW_BTN_A)){ //Check if button A is pressed.
          nav_get_loc(&row, &column); // Get selected position

          if (board_set(row, column, current_player)) {  // If its a valid move
          int8_t location_byte = (row << NUM_BIT_SHIFT) | (column & NUM_MASK); 
          com_write(&location_byte, sizeof(location_byte));
          current_state = MARK_ST;  // Transition to MARK_ST
        }
      }

      if (com_read(&incoming_byte, sizeof(incoming_byte)) > 0){ 
        row = (incoming_byte >> NUM_BIT_SHIFT) & NUM_MASK; 
        column = incoming_byte & NUM_MASK;
        if (board_set(row,column,current_player)){
          current_state= MARK_ST;
        }
      }
      break;

    case MARK_ST:
      if (board_winner(current_player)) {  // Check for a win
          current_state = WAIT_RESTART_ST;  // Game over, wait for restart
      } 
      else if (board_mark_count() == CONFIG_BOARD_SPACES) {  // Check for draw
          current_state = WAIT_RESTART_ST;  // Game over, wait for restart
      } 
      else {
          current_state = WAIT_MARK_ST;  // Continue game
      }
      break;

    case WAIT_RESTART_ST:
      if (!pin_get_level(HW_BTN_START)){ //Check if button START is pressed.
          current_state = NEW_GAME_ST; // Restart the game
      }
      break;
      
    default:
      current_state = INIT_ST;
      break;
  }



  //switch for State Machine Actions
  switch (current_state) { 
      
    case INIT_ST: //No actions needed
        break;
    
    case NEW_GAME_ST:
      board_clear();          // Reset the board
      while (com_read(&incoming_byte, sizeof(incoming_byte)) > 0) {
       // This will discard any stray bytes
    }
    
      board_clear();          // Reset the board
      lcd_fillScreen(CONFIG_BACK_CLR); //draw background color
      graphics_drawGrid(CONFIG_GRID_CLR);    //draw grid
      current_player = 'X';   // X starts the game
      nav_set_loc(1, 1);             // Move navigator to center of grid
      lcd_drawRect2(0,LCD_H-LCD_CHAR_H,LCD_W,LCD_H,CONFIG_BACK_CLR);
      lcd_fillRect2(0,LCD_H-LCD_CHAR_H,LCD_W,LCD_H,CONFIG_BACK_CLR); //clear text area
      lcd_drawString(0,LCD_H-LCD_CHAR_H,"Player X's Turn", CONFIG_MESS_CLR);
      break;

    case WAIT_MARK_ST: //No actions needed
      break;
    
    case MARK_ST:
      //Place the mark if the highlighted space is empty
      mark_t current_mark;
      if (current_player == 'X'){
          current_mark = X_m;
      }
      else{
        current_mark = O_m;
      }
        
      if (current_player == 'X'){
        graphics_drawX(row, column, CONFIG_MARK_CLR);
      }
      else{
        graphics_drawO(row, column, CONFIG_MARK_CLR);
      }
      

      //board_mark_count ++; do the functions do this automatically for me

      if (board_winner(current_player)) { // Check for a win
        char message[MESSAGE_SIZE];
        sprintf(message, "Player %c Wins!", current_player); 
        lcd_drawRect2(0,LCD_H-LCD_CHAR_H,LCD_W,LCD_H,CONFIG_BACK_CLR);
        lcd_fillRect2(0,LCD_H-LCD_CHAR_H,LCD_W,LCD_H,CONFIG_BACK_CLR); //clear text area
        lcd_drawString(0,LCD_H-LCD_CHAR_H,message, CONFIG_MESS_CLR);
        //lcd_drawString();
        game_over = true;
        current_state = WAIT_RESTART_ST; // Wait for restart
      } 
      else if (board_mark_count()== CONFIG_BOARD_SPACES){//CONFIG_BOARD_SPACES) { // If board is full but no winner
        lcd_drawRect2(0,LCD_H-LCD_CHAR_H,LCD_W,LCD_H,CONFIG_BACK_CLR);
        lcd_fillRect2(0,LCD_H-LCD_CHAR_H,LCD_W,LCD_H,CONFIG_BACK_CLR); //clear text area
        lcd_drawString(0,LCD_H-LCD_CHAR_H,"It's a draw!", CONFIG_MESS_CLR);
        game_over = true;
        current_state = WAIT_RESTART_ST; // Wait for restart
      } 
      else {
        // No win or draw, switch player and continue
        switch_player();
        char message[MESSAGE_SIZE];
        sprintf(message, "Player %c's Turn", current_player); 
        lcd_drawRect2(0,LCD_H-LCD_CHAR_H,LCD_W,LCD_H,CONFIG_BACK_CLR);
        lcd_fillRect2(0,LCD_H-LCD_CHAR_H,LCD_W,LCD_H,CONFIG_BACK_CLR); //clear text area
        lcd_drawString(0,LCD_H-LCD_CHAR_H,message, CONFIG_MESS_CLR);
      }
      break;

    case WAIT_RESTART_ST: //no actions needed
      break; 

    default:
      break;        
  }
}

