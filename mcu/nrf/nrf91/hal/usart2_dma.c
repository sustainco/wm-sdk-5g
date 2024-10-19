#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "board.h"
#include "hal_api.h"
#include "api.h"

#define DEBUG_LOG_MODULE_NAME "USART M"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO
#include "debug_log.h"

/* Private function declarations */
static bool usart2_set_baud(uart_t *self);
static int usart2_init_dma(uart_t *self);
static int usart2_start_tx_lock(uart_t *self);
static int usart2_set_rx(uart_t *self);

/* Method implementations */
static int usart2_init(uart_t *self, uart_cfg_t cfg) {

    if(self == NULL){
        return UART_OP_CONFIG_ERROR;
    }

    self->cfg = cfg;

    nrf_gpio_cfg_default(self->cfg.tx_pin);
    nrf_gpio_pin_set(self->cfg.tx_pin);
    // nrf_gpio_cfg_default(self->cfg.rx_pin);
    // nrf_gpio_pin_set(self->cfg.rx_pin);

    self->cfg.uart->ENABLE = UARTE_ENABLE_ENABLE_Disabled;
    self->cfg.uart->PSEL.TXD = self->cfg.tx_pin;
    self->cfg.uart->PSEL.RXD = self->cfg.rx_pin;
    self->cfg.uart->TASKS_STOPTX = 1;
    self->cfg.uart->TASKS_STOPRX = 1;

    if (!usart2_set_baud(self)) {
        return UART_OP_CONFIG_ERROR;
    }

    if (usart2_init_dma(self) != UART_OP_OK) {
        return UART_OP_CONFIG_ERROR;
    }

    nrf_gpio_cfg(self->cfg.tx_pin,
                 NRF_GPIO_PIN_DIR_OUTPUT,
                 NRF_GPIO_PIN_INPUT_CONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_S0S1,
                 NRF_GPIO_PIN_NOSENSE);

    

    self->cfg.uart->CONFIG = 0;

    if (usart2_set_rx(self) != UART_OP_OK) {
        return UART_OP_CONFIG_ERROR;
    }

    self->cfg.uart->ENABLE = UARTE_ENABLE_ENABLE_Enabled;

    return UART_OP_OK;
}

static int usart2_set_rx(uart_t *self) {

    Sys_enterCriticalSection();

    nrf_gpio_cfg(self->cfg.rx_pin,
                 NRF_GPIO_PIN_DIR_INPUT,
                 NRF_GPIO_PIN_INPUT_CONNECT,
                 NRF_GPIO_PIN_PULLUP,
                 NRF_GPIO_PIN_S0S1,
                 NRF_GPIO_PIN_SENSE_LOW);

    self->cfg.uart->EVENTS_RXDRDY = 0;
    self->cfg.uart->EVENTS_ENDRX = 0;
    self->cfg.uart->EVENTS_RXSTARTED = 0;

    self->cfg.uart->RXD.PTR = (uint32_t) self->rx_buffer;
    self->cfg.uart->RXD.MAXCNT = UART_BUFFER_SIZE;

    Sys_exitCriticalSection();

    return UART_OP_OK;
}

static int usart2_clear_buffer(uart_t *self) {

    Sys_enterCriticalSection();

    self->cfg.uart->TASKS_STOPRX = 1;

    while (self->cfg.uart->EVENTS_RXTO != 1) {}

    memset(self->rx_buffer, 0, sizeof(self->rx_buffer));

    self->cfg.uart->RXD.PTR = (uint32_t) self->rx_buffer;
    self->cfg.uart->RXD.MAXCNT = UART_BUFFER_SIZE;

    self->cfg.uart->TASKS_STARTRX = 1;

    while (self->cfg.uart->EVENTS_RXSTARTED != 1) {}

    Sys_exitCriticalSection();

    return UART_OP_OK;
}

static unsigned int usart2_tx(uart_t *self, const void *buffer, unsigned int length) {

    Sys_enterCriticalSection();

    if (usart2_clear_buffer(self) != UART_OP_OK) {
        Sys_exitCriticalSection();
        return 0;
    }

    if(length > UART_BUFFER_SIZE){
        Sys_exitCriticalSection();
        return 0;
    }
    memcpy(self->tx_buffer.buffer,
           buffer,
           length);

    self->tx_buffer.index = length;

    int ret = usart2_start_tx_lock(self);

    Sys_exitCriticalSection();

    return ret;
}

static int usart2_rx(uart_t *self, uint8_t *data, unsigned int len) {

    Sys_enterCriticalSection();

    self->cfg.uart->TASKS_STOPRX = 1;

    while (self->cfg.uart->EVENTS_ENDRX != 1) {}

    int bytes = (int) strlen((const char*) self->rx_buffer);

    if (bytes == 0) {
        Sys_exitCriticalSection();
        return UART_OP_NO_DATA;
    }

    memcpy(data, self->rx_buffer, bytes);

    self->cfg.uart->TASKS_STARTRX = 1;

    while (self->cfg.uart->EVENTS_RXSTARTED != 1) {}

    Sys_exitCriticalSection();

    return bytes;
}

static bool usart2_set_baud(uart_t *self) {

    switch (self->cfg.baudrate) {
        case 115200:
            self->cfg.uart->BAUDRATE = (uint32_t) UARTE_BAUDRATE_BAUDRATE_Baud115200;
            break;
        case 125000:
            self->cfg.uart->BAUDRATE = (uint32_t)(0x02000000UL);
            break;
        case 250000:
            self->cfg.uart->BAUDRATE = (uint32_t) UARTE_BAUDRATE_BAUDRATE_Baud250000;
            break;
        case 460800:
            self->cfg.uart->BAUDRATE = (uint32_t) UARTE_BAUDRATE_BAUDRATE_Baud460800;
            break;
        case 1000000:
            self->cfg.uart->BAUDRATE = (uint32_t) UARTE_BAUDRATE_BAUDRATE_Baud1M;
            break;
        case 2400:
            self->cfg.uart->BAUDRATE = (uint32_t) UARTE_BAUDRATE_BAUDRATE_Baud2400;
            break; 
        case 9600:
            self->cfg.uart->BAUDRATE = (uint32_t) UARTE_BAUDRATE_BAUDRATE_Baud9600;
            break; 
        default:
            return false;
    };

    return true;
}

static int usart2_init_dma(uart_t *self) {

    self->cfg.uart->INTENCLR = 0xffffffffUL;

    self->cfg.uart->INTENSET =
        (UARTE_INTEN_ENDTX_Enabled << UARTE_INTEN_ENDTX_Pos) |
        (UARTE_INTEN_ERROR_Enabled << UARTE_INTEN_ERROR_Pos);

    return UART_OP_OK;
}

static int usart2_start_tx_lock(uart_t *self) {

    self->cfg.uart->TXD.PTR = (uint32_t) self->tx_buffer.buffer;
    self->cfg.uart->TXD.MAXCNT = self->tx_buffer.index;

    self->cfg.uart->EVENTS_ENDTX = 0;
    self->cfg.uart->TASKS_STARTTX = 1;

    while (self->cfg.uart->EVENTS_ENDTX == 0) {}

    return self->cfg.uart->TXD.AMOUNT;
}

/* Function to initialize method pointers */
void uart_create(uart_t *self) {

    self->init = usart2_init;
    self->set_rx = usart2_set_rx;
    self->tx = usart2_tx;
    self->rx = usart2_rx;
    self->clear_buffer = usart2_clear_buffer;

}