#include <stdio.h>
#include "soc/reg_base.h" // DR_REG_GPIO_BASE, DR_REG_IO_MUX_BASE
#include "driver/rtc_io.h" // rtc_gpio_*
#include "pin.h"

//GPIO Matrix Registers 
#define GPIO_OUT_REG          (DR_REG_GPIO_BASE+0x04)  
#define GPIO_OUT1_REG 		  (DR_REG_GPIO_BASE+0x10)  
#define GPIO_OUT_W1TS_REG	  (DR_REG_GPIO_BASE+0x08)
#define GPIO_OUT_W1TC_REG	  (DR_REG_GPIO_BASE+0x0C)
#define GPIO_OUT1_W1TS_REG	  (DR_REG_GPIO_BASE+0x14)
#define GPIO_OUT1_W1TC_REG	  (DR_REG_GPIO_BASE+0x18)
#define GPIO_IN_REG 		  (DR_REG_GPIO_BASE+0X3C) 
#define GPIO_IN1_REG 		  (DR_REG_GPIO_BASE+0x40)  

#define GPIO_ENABLE_REG 	  (DR_REG_GPIO_BASE+0x20) 
#define GPIO_ENABLE_W1TS_REG  (DR_REG_GPIO_BASE+0x24) 
#define GPIO_ENABLE_W1TC_REG  (DR_REG_GPIO_BASE+0x28) 
#define GPIO_ENABLE1_REG	  (DR_REG_GPIO_BASE+0x2C) 
#define GPIO_ENABLE1_W1TS_REG (DR_REG_GPIO_BASE+0x30)  
#define GPIO_ENABLE1_W1TC_REG (DR_REG_GPIO_BASE+0x34)  

#define GPIO_PINN_REG(n)		      (DR_REG_GPIO_BASE+0x88+0x4*(n)) 
#define GPIO_FUNCN_OUT_SEL_CFG_REG(n) (DR_REG_GPIO_BASE+0x530+0x4*(n)) 

//IO_MUX registers 
#define IO_MUX_REG(n)         (DR_REG_IO_MUX_BASE + PIN_MUX_REG_OFFSET[n])

#define FUN_WPD  7 	//bit for pull-down enable
#define FUN_WPU 8 	//bit for pull-up enable
#define FUN_IE 9 	//bit for input enable
#define FUN_DRV 11 	//bit for driver
#define MCU_SEL 13 	//bit for MCU selection

//Memory location registers 
#define PAD_DRIVER	  			(2) //memory location of PAD_DRIVER within GPIO_PINN_REG
#define GPIO_FUNCN_OUT_SEL		(0x100) //memory location within GPIO_FUNCN_OUT_SEL_CFG_REG

//this value sets MCU_SEL=2, FUN_DRV=2, FUN_WPU=1
#define IO_MUX_REG_BITMASK (0x2900) 

//Operations registers
#define REG(r) (*(volatile uint32_t *)(r))  //r is address of register
#define REG_BITS 32
#define REG_SET_BIT(r,b) (REG(r)|=(0x01<<b))// b is the bit in the register that we are setting to 
#define REG_CLR_BIT(r,b) (REG(r) &= ~(0x01 << b))
#define REG_GET_BIT(r,b) ((REG(r) & (0x01 << b)) != 0) //((REG(r) >> (b)) &1)
#define REG_CLR_REG(r) 	(REG(r) = 0x0)

// Gives byte offset of IO_MUX Configuration Register
// from base address DR_REG_IO_MUX_BASE
static const uint8_t PIN_MUX_REG_OFFSET[] = {
    0x44, 0x88, 0x40, 0x84, 0x48, 0x6c, 0x60, 0x64, // pin  0- 7
    0x68, 0x54, 0x58, 0x5c, 0x34, 0x38, 0x30, 0x3c, // pin  8-15
    0x4c, 0x50, 0x70, 0x74, 0x78, 0x7c, 0x80, 0x8c, // pin 16-23
    0x90, 0x24, 0x28, 0x2c, 0xFF, 0xFF, 0xFF, 0xFF, // pin 24-31
    0x1c, 0x20, 0x14, 0x18, 0x04, 0x08, 0x0c, 0x10, // pin 32-39
};

// Reset the configuration of a pin to not be an input or an output.
// Pull-up is enabled so the pin does not float.
int32_t pin_reset(pin_num_t pin)
{
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		rtc_gpio_deinit(pin);
		rtc_gpio_pullup_en(pin);
		rtc_gpio_pulldown_dis(pin);
	} 
    
    //Reset GPIO_PINn_REG: All fields zero
	REG_CLR_REG(GPIO_PINN_REG(pin));

	//Reset GPIO_FUNCn_OUT_SEL_CFG_REG: GPIO_FUNCn_OUT_SEL=0x100
	REG(GPIO_FUNCN_OUT_SEL_CFG_REG(pin)) = GPIO_FUNCN_OUT_SEL; 

	//Reset IO_MUX_x_REG: MCU_SEL=2, FUN_DRV=2, FUN_WPU=1; 
	REG(IO_MUX_REG(pin)) = IO_MUX_REG_BITMASK;  //0010 1001 0000 0000 or 0x2900
	// NOTE: By default, pin should not float, save power with FUN_WPU=1
	
	// Now that the pin is reset, set the output level to zero
	return pin_set_level(pin, 0);
}

// Enable or disable a pull-up on the pin.
int32_t pin_pullup(pin_num_t pin, bool enable)
{
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		if (enable) return rtc_gpio_pullup_en(pin);
		else return rtc_gpio_pullup_dis(pin);
	}
	//Set or clear the FUN_WPU bit in an IO_MUX register
	if(enable){ //set FUN_WPU bit if enable
		REG_SET_BIT(IO_MUX_REG(pin), FUN_WPU);
	}
	else{ // clr FUN_WPU bit if not enabled
		REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPU);
	}
	return 0;
}

// Enable or disable a pull-down on the pin.
int32_t pin_pulldown(pin_num_t pin, bool enable)
{
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		if (enable) return rtc_gpio_pulldown_en(pin);
		else return rtc_gpio_pulldown_dis(pin);
	}
	// Set or clear the FUN_WPD bit in an IO_MUX register
	if(enable){ //Set FUN_WPD bit if enabled
		REG_SET_BIT(IO_MUX_REG(pin), FUN_WPD);
	}
	else{ //Clear FUN_WPD bit if not enabled
		REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPD);
	}
	return 0;
}

// Enable or disable the pin as an input signal.
int32_t pin_input(pin_num_t pin, bool enable)
{
	// Set or clear the FUN_IE bit in an IO_MUX register
	if(enable){ //Set FUN_IE bit if enabled
		REG_SET_BIT(IO_MUX_REG(pin), FUN_IE);
	}
	else{ //Clear FUN_IE bit if not enabled
		REG_CLR_BIT(IO_MUX_REG(pin), FUN_IE);
	}
	return 0;
}

// Enable or disable the pin as an output signal.
int32_t pin_output(pin_num_t pin, bool enable)
{
	// Set or clear the I/O pin bit in the ENABLE or ENABLE1 register
	if (enable){	
		if (pin < (REG_BITS)){ //Set the I/O pin bit in enable
			REG_SET_BIT(GPIO_ENABLE_W1TS_REG, pin); 
		}
		else{ //set the I/O pin bit in enable1
			REG_SET_BIT(GPIO_ENABLE1_W1TS_REG, (pin-REG_BITS)); 
		}
	}
	else{
		if (pin < (REG_BITS)){  //Clear the I/O pin bit in enable
			REG_SET_BIT(GPIO_ENABLE_W1TC_REG, pin); 
		}
		else{ //clear the I/O pin bit in enable1
			REG_SET_BIT(GPIO_ENABLE1_W1TC_REG, (pin-REG_BITS)); 
		}
	}
	return 0;
}

// Enable or disable the pin as an open-drain signal.
int32_t pin_odrain(pin_num_t pin, bool enable)
{
	// Set or clear the PAD_DRIVER bit in a PIN register
	if(enable){ //set the pad driver bit in a pin register
		REG_SET_BIT(GPIO_PINN_REG(pin),PAD_DRIVER);
	}
	else{ //clear the pad driver bit in a pin register
		REG_CLR_BIT(GPIO_PINN_REG(pin),PAD_DRIVER);
	}
	return 0;
}

// Sets the output signal level if the pin is configured as an output.
int32_t pin_set_level(pin_num_t pin, int32_t level)
{
	if (level){	//if level set the output signal 
		if (pin < (REG_BITS)){ //for pins 0 through 31
			return REG_SET_BIT(GPIO_OUT_REG,pin); 
		}
		else{ //for pins 32-38
			return REG_SET_BIT(GPIO_OUT1_REG, (pin-REG_BITS)); 
		}
	}
	else{ //if not level clear the output signal 
		if (pin < (REG_BITS)){  //for pins 0 through 31 
			return REG_CLR_BIT(GPIO_OUT_REG,pin); 
		}
		else{ //for pins 32-38 
			return REG_CLR_BIT(GPIO_OUT1_REG, (pin-REG_BITS)); 
		}
	}
	return 0;
}

// Gets the input signal level if the pin is configured as an input.
int32_t pin_get_level(pin_num_t pin)
{
	// Get the I/O pin bit from the IN or IN1 register
	if (pin < (REG_BITS)){ //Get the I/O pin bit for IN register
		return REG_GET_BIT(GPIO_IN_REG, pin); 
	}
	else{ //Get the I/O pin bit for IN1 Register
		return REG_GET_BIT(GPIO_IN1_REG, (pin-REG_BITS)); 
	}
	return 0;
}

// Get the value of the input registers, one pin per bit.
// The two 32-bit input registers are concatenated into a uint64_t.
uint64_t pin_get_in_reg(void)
{
	// Read the IN and IN1 registers, return the concatenated values
	uint64_t all_in_pins; 
	all_in_pins = ((uint64_t)(REG(GPIO_IN1_REG)) << REG_BITS) | REG(GPIO_IN_REG);
	return all_in_pins;
}

//Concatenate the two out registers into a uint64_t 
uint64_t pin_get_out_reg(void)
{ 
	uint64_t all_out_pins;  
	all_out_pins = ((uint64_t)(REG(GPIO_OUT1_REG)) << REG_BITS) | REG(GPIO_OUT_REG);
	return all_out_pins;
}
