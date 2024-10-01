#ifndef USART2_H_
#define USART2_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef enum{
    UART2_FLOW_CONTROL_NONE,
    UART2_FLOW_CONTROL_HW,
} uart2_flow_control_e;


typedef enum {
    UART_OP_CONFIG_ERROR = -4,
    UART_OP_NO_DATA,
    UART_OP_DATA_ERROR,
    UART_OP_ERROR,
    UART_OP_OK
}uart_op_t;

/**
 * @brief Initialize the UART port
 * 
 * @param baudrate 
 * @param flow_control 
 * @return int 
 */
int usart2_init(uint32_t baudrate, unsigned int flow_control);

/**
 * @brief Initialize RX and set the related callback
 * 
 * @return int operation error or the number of bytes received
 */
int usart2_set_rx(void);

/**
 * @brief 
 * 
 * @param buf 
 * @param len 
 * @return uint32_t 
 */
uint32_t usart2_tx(const void * buf, uint32_t len);

/**
 * @brief 
 * 
 * @param data 
 * @param len 
 * @return int 
 */
int usart2_rx(uint8_t* data, unsigned int len);


#endif /* USART2_H_ */
