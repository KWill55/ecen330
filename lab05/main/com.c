#include <driver/uart.h>

#include "hw.h"
#include "pin.h"
#include "com.h"

#define BAUD_RATE 115200

int32_t com_init(void){
    //Use UART_NUM_2 for the port number
    const uart_port_t uart_num = UART_NUM_2;

    //Set up UART 
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
    
    
    //UART transmit and receive pins
    // Set UART pins(TX: IO4, RX: IO5, RTS: IO18, CTS: IO19)
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2,HW_EX8, HW_EX7, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    //TODO change above comment 


    //install the driver
    // Setup UART buffered IO with event queue
    const int rx_uart_buffer_size = UART_HW_FIFO_LEN(PORT_NUM)*2;
    const int tx_uart_buffer_size = 0;
    QueueHandle_t uart_queue;
    // Install UART driver using an event queue here
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, rx_uart_buffer_size, tx_uart_buffer_size, 0, NULL, 0));


    //configure pin 7 
    pin_pullup(HW_EX7, true); //pin
    
    return 0;
}


int32_t com_deinit(void){
    //delete the UART driver if it has been installed
    uart_driver_delete(UART_NUM_2);
    return 0;
}

//function that writes to the second board 
int32_t com_write(const void *buf, uint32_t size){
    return uart_tx_chars(UART_NUM_2, (const char*)buf, size);
}


//function that reads the second board 
int32_t com_read(void *buf, uint32_t size){
    return uart_read_bytes(UART_NUM_2, (uint8_t*)buf, size, 0);
}