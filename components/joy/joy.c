/*
Creating the cursor is taking analog values and converting them to digital
Creating the sounds is taking digital values and converting them to analog
*/

/*
joy.c does ??? 
*/

#include <stdio.h>
#include "esp_adc/adc_oneshot.h"
#include "joy.h"

//Declare Macro Variables 
#define NUM_READS 5 //how many reads we want to average to get the center cursor position

//Declare Global Variables  
int64_t joy_x_center = 0; 
int64_t joy_y_center = 0;
adc_oneshot_unit_handle_t adc1_handle; //ADC Unit 1 handle

/*
This function configures ADC and finds the center position of the cursor
*/
int32_t joy_init(void){ 
    //configure an ADC one-shot unit
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1, //Use ADC Unit 1
        .ulp_mode = ADC_ULP_MODE_DISABLE, //ULP mode disabled
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,//default bitwdith
        .atten = ADC_ATTEN_DB_12,  //attenuate input by 12 dB
    };
    
    //Configure ADC1 channels 6 and 7 
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_7, &config));

    //find the center position of the joystick by averaging a few reads from the ADC. 
    int_fast32_t x_reading = 0; 
    int_fast32_t y_reading = 0; 
    int64_t x_readings[NUM_READS]; 
    int64_t y_readings[NUM_READS]; //TODO change these ints 

    for (int8_t i = 0; i< NUM_READS; i++){ //this loop gets readings for x and y 
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &x_reading));
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_7, &y_reading));
        x_readings[i] = x_reading;
        y_readings[i] = y_reading;
    }

    float x_avg = 0.0;
    float y_avg = 0.0;

    for (int i=0; i< NUM_READS; i++){
        x_avg += x_readings[i];
        y_avg += y_readings[i];
    }

    x_avg /= NUM_READS;
    y_avg /= NUM_READS;
    
    //Save this globally for later use in calculating the joystick displacement 
    joy_x_center = x_avg;  
    joy_y_center = y_avg; 
    return 0;
}


/*
this function deletes ADC Unit 1 if its hanndle is not NULL
*/
int32_t joy_deinit(void){
    //delete ADC Unit 1 if its hanndle is not NULL
    if  (adc1_handle != NULL){
        adc_oneshot_del_unit(adc1_handle);
    }
    return 0;
}

/*
displacement of cursor x and displacement of cursor y 
Read the ADC using adc_oneshot_read() to determine the joystick position. 
Read it once for the x channel (ADC_CHANNEL_6) and once for the y channel(ADC_CHANNEL_7)
*/
void joy_get_displacement(int32_t *dcx, int32_t *dcy){ 
    int_fast32_t joy_x = 0;
    int_fast32_t joy_y = 0;

    //get the current x and y positions of the cursor 
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &joy_x)); 
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_7, &joy_y)); 
    //Calculate the joystick displacement relative to the center postiion (saved during Init)
    *dcx = (joy_x -joy_x_center); 
    *dcy = (joy_y -joy_y_center); 
}
