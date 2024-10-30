#include <stdio.h>

#include "driver/gptimer.h"
#include "hw.h" // defines I/O pins associated with buttons
#include "lcd.h"
#include "pin.h"
#include "watch.h"
#include "esp_timer.h"
#include "esp_log.h"

//Declare Macro Variables
#define RESOLUTION_HZ_MEG 1000000 //1 MegHz resolution 
#define ALARM_PERIOD 10000 //Timer should alarm every 10ms (1/100th sec)
#define ISR_TIMING 500 //should display the ISR max time value every 5 seconds

//Declare Global Variables
volatile uint64_t timer_ticks = 0;
volatile bool running_flag = false; 
volatile int64_t isr_max; // Maximum ISR execution time (us)
volatile int32_t isr_cnt; // Count of ISR invocations 
static const char *TAG = "lab03";
int64_t start = 0;
int64_t finish = 0;

/*
This function detects button presses and updates timer_ticks counter accordingly. 
This is our ISR function.It also keeps track of the isr_max time. 
*/
static bool timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx){
    start = esp_timer_get_time();
    if (!pin_get_level(HW_BTN_A)){ //Check if button A is pressed. If so, set the running flag to true.
        running_flag = true;
    }
    if (!pin_get_level(HW_BTN_B)){ //Check if button B is pressed. If so, set the running flag to false.
        running_flag = false;
    }
    if (!pin_get_level(HW_BTN_START)){//Check if button START is pressed. If so, set the running flag to false and set timer_ticks to zero.
        running_flag = false;
        timer_ticks = 0; 
    }
    if (running_flag == true){//If the running flag is true, increment timer_ticks.
        timer_ticks ++;
    }
    isr_cnt++;
    finish = esp_timer_get_time();
    if (finish-start > isr_max){ //replace isr_max if it is larger than before
        isr_max = finish-start;
    }
    return false;
}

/*
Main application: configure timer, action, and start/run stopwatch
*/
void app_main(void){ //main application
//configure I/O pins for buttons using pin_ functions
    start = esp_timer_get_time();
    //Button A starts the stopwatch
    pin_reset(HW_BTN_A);
    pin_input(HW_BTN_A,true);
    //Button B stops the stopwatch
    pin_reset(HW_BTN_B);
    pin_input(HW_BTN_B,true);
    //Button Start resets stopwatch to zero 
    pin_reset(HW_BTN_START);
    pin_input(HW_BTN_START,true);
    //report time after setting pins:
    finish = esp_timer_get_time();
    printf("Setting pin time:%lld microseconds\n", finish-start);

//create a new timer instance (GPTIMER handle)
    start = esp_timer_get_time();
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = { //configure timer
    .clk_src = GPTIMER_CLK_SRC_DEFAULT, //default clock source
    .direction = GPTIMER_COUNT_UP, //count up 
    .resolution_hz = RESOLUTION_HZ_MEG, // 1MHz resolution, 1 tick = 1us
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
//register the timer callback function
    gptimer_event_callbacks_t cbs = { //register user callback
    .on_alarm = timer_callback, 
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));  
    ESP_ERROR_CHECK(gptimer_enable(gptimer)); //enable the timer 
    ESP_ERROR_CHECK(gptimer_start(gptimer));  //start the timer
    finish = esp_timer_get_time();
    printf("Timer configuration time:%lld microseconds\n", finish-start);
//set up alarm action
    gptimer_alarm_config_t alarm_config = { //configure alarm action
    .reload_count = 0, // counter will reload with 0 on alarm event
    .alarm_count = ALARM_PERIOD, // period = 10ms @resolution 1MHz (1/100th sec)
    .flags.auto_reload_on_alarm = true, // enable auto-reload
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
//update the time on the display 
    start = esp_timer_get_time();
    ESP_LOGI(TAG, "Stopwatch update");
    finish = esp_timer_get_time();
    printf("ESP_LOGI function time:%lld microseconds\n", finish-start);
    lcd_init(); // Initialize LCD display
    watch_init(); // Initialize stopwatch face
    for (;;) { // forever update loop
        watch_update(timer_ticks); 
        if (isr_cnt >= ISR_TIMING){ //print the maximum ISR execution time every 5 seconds
            printf("Previous 5 seconds ISR Max:%lld microseconds\n", isr_max);
            isr_max = 0;
            isr_cnt = 0;
        }
    }
}




