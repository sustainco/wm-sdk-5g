#ifndef _UART_H_
#define _UART_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include "hal_api.h"


#define UART_BUFFER_SIZE 256U
/**
 * @brief Flow control states
 */
typedef enum{
    UART2_FLOW_CONTROL_NONE,
    UART2_FLOW_CONTROL_HW,
} uart2_flow_control_e;

/**
 * @brief UART operation errors
 * 
 */
typedef enum {
    UART_OP_CONFIG_ERROR = -4,
    UART_OP_NO_DATA,
    UART_OP_DATA_ERROR,
    UART_OP_ERROR,
    UART_OP_OK
}uart_op_t;

typedef struct uart_cfg{
    /* communication baud rate */
    int baudrate;
    /* UART port to use */
    NRF_UARTE_Type* uart;
    /* receive pin */
    unsigned int rx_pin;
    /* Transmit pin */
    unsigned int tx_pin;
    /* UART flow control */
    unsigned int flow_control;
}uart_cfg_t;

/* Transmit buffer object */
typedef struct {
    uint8_t buffer[UART_BUFFER_SIZE];
    int index;
}tx_buffer_t;

typedef struct uart_t uart_t;
struct uart_t {
    /* UART configuration*/
    uart_cfg_t cfg;

    /* UART receive buffer*/
    uint8_t rx_buffer[UART_BUFFER_SIZE];

    /* UART transmit buffer */
    tx_buffer_t tx_buffer;

    /**
     * @brief initialize the UART port
     * @param self uart object
     * @param cfg uart configuration
     * @return int operaton status
     */
    int (*init)(struct uart_t *self, uart_cfg_t cfg);

    /**
     * @brief set the receive configuration of the UART
     * @param self uart object
     * @return int operation status
     */
    int (*set_rx)(struct uart_t *self);

    /**
     * @brief transmit data through UART port
     * @param self uart object
     * @param buf buffer containing data to transmit
     * @param len length of the data to transmit
     * @return error or number of bytes transmitted
     * 
     */
    unsigned int (*tx)(struct uart_t *self, const void *buf, unsigned int len);

    /**
     * @brief Receive data though the UART port
     * @param self uart object
     * @param data buffer to store the received data
     * @param len size of the receive buffer
     * @return int error or number of bytes received
     * 
     */
    int (*rx)(struct uart_t *self, uint8_t *data, unsigned int len);

    /**
     * @brief Clear the uart receive buffer
     * @param self uart object
     * @return int operation status
     */
    int (*clear_buffer)(struct uart_t *self);

};

/* Create a UART port */
void uart_create(uart_t *self);


#endif // _UART_H_
