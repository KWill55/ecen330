/*
Creating the cursor is taking analog values and converting them to digital
Creating the sounds is taking digital values and converting them to analog
*/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "tone.h"
#include "sound.h"

#define BIAS_LEVEL 0x80
#define BYTE_MAX 0xFF 
#define SAMPLE_RATE 24000

#define DOUBLE 2
#define HALVE 2
#define HALF 0.5
#define NYQUIST_RATIO 2
#define FIRST_QUARTER 0.25
#define THIRD_QUARTER 0.75
#define OFFSET 5

static uint8_t *toneWaveform_buffer = NULL;

// Initialize the tone driver. Must be called before using.
// May be called again to change sample rate.
// sample_hz: sample rate in Hz to playback tone.
// Return zero if successful, or non-zero otherwise.
int32_t tone_init(uint32_t sample_hz){
    uint32_t nyquist_hz = sample_hz/HALVE; //nyquist frequency should be half of the sample rate frequency
    
    if ((sample_hz/nyquist_hz)<NYQUIST_RATIO) {// check the sample rate parameter to make sure it is at least twice the lowest Nyquist frequency.
        printf("Error: Sample rate is not twice the size of nyquist frequency.\n");
        return -1;
    }
    
    sound_init(sample_hz); //Call sound_init() from the sound component (provided in the starting code).
    
    uint32_t samples_per_period = sample_hz/LOWEST_FREQ;
    toneWaveform_buffer = (uint8_t *)malloc(samples_per_period * sizeof(uint8_t));//allocate global buffer for tone waveforms
    return 0;
}


// Free resources used for tone generation (DAC, etc.).
// Return zero if successful, or non-zero otherwise.
int32_t tone_deinit(void){
    free(toneWaveform_buffer); //Free the waveform buffer allocated during init.
    sound_deinit(); //Call sound_deinit().
    return 0; 
}


// Start playing the specified tone.
// tone: one of the enumerated tone types.
// freq: frequency of the tone in Hz.
void tone_start(tone_t tone, uint32_t freq){
    //Check the tone to see if is within bounds.
    
    //check to see if it is a valid waveform
    if (tone != SINE_T && tone != SQUARE_T && 
        tone != TRIANGLE_T && tone != SAW_T) {
        printf("Error: Invalid tone.\n");
        return;
        }

    //Check the frequency to see if is within bounds.
    if (freq < LOWEST_FREQ){
        printf("Frequency is too low");
        return;
    }

    uint32_t samples_per_period = SAMPLE_RATE/freq; //Calculate the number of samples in one period for the specified frequency (DAC sample rate / waveform frequency).
    
    
    //Generate one cycle of the specified waveform into the allocated buffer. The element type of the buffer should be uint8_t. 
    switch (tone) {
        case SINE_T: //Sine wave 
            for (int16_t i = 0; i < samples_per_period; i++) { //i is how far along we are in the x axis
                float phase = (float)i/samples_per_period;
                toneWaveform_buffer[i] = (uint8_t)(BIAS_LEVEL + (BIAS_LEVEL - 1) * sinf(DOUBLE * M_PI * phase));
            }
            break;
        case SQUARE_T: //Square wave 
            for (int16_t i = 0; i < samples_per_period; i++) { //i is how far along we are in the x axis
                if (i < samples_per_period / HALVE) { //if its the first half of the period
                    toneWaveform_buffer[i] = BYTE_MAX; // Maximum value 
                } else { //if i is in the second half of the period 
                    toneWaveform_buffer[i] = 0x00; // Minimum value 
                }
            }
            break;
        
        case TRIANGLE_T: // Triangle wave 
            for (int16_t i = 0; i < samples_per_period; i++) { // Loop through each sample in the period
                float phase = (float)i / samples_per_period; // Calculate the current phase as a fraction of the period

                if (phase < FIRST_QUARTER) { // For the first half of the period (ascending part of the triangle)
                    toneWaveform_buffer[i] = (uint8_t)(BIAS_LEVEL + DOUBLE * (BYTE_MAX - BIAS_LEVEL) * phase*DOUBLE);
                } else if (phase < THIRD_QUARTER) { // For the second half (descending part of the triangle)
                    toneWaveform_buffer[i] = (uint8_t)(BIAS_LEVEL + DOUBLE * (BYTE_MAX - BIAS_LEVEL) * (1 - (phase - HALF) * DOUBLE));
                } else { // Going up the on the last part of the period
                    toneWaveform_buffer[i] = (uint8_t)(DOUBLE * (BYTE_MAX - BIAS_LEVEL) * phase*DOUBLE - BIAS_LEVEL +OFFSET);
                }
            }
            break;

        case SAW_T: //Sawtooth wave 
            for (int16_t i = 0; i < samples_per_period; i++) {
                float phase = (float)i / samples_per_period;
                toneWaveform_buffer[i] = (uint8_t)((BYTE_MAX - BIAS_LEVEL) * (DOUBLE * phase - 1));
            }
            break;
        default:
            printf("Error: Your waveform was not valid\n");
            break;
    }

    sound_cyclic(toneWaveform_buffer, samples_per_period); //Call sound_cyclic() to play the generated waveform.
}