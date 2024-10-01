#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "board.h"
#include "hal_api.h"
#include "api.h"

/** Declare buffer size (defined by max DMA transfert size) */
#define BUFFER_SIZE 256u
#include "doublebuffer.h"

/* Size of the RX Buffer */
#define SYS_RX_BUFFER_SIZE 256
/* Buffer to receive UART bytes */
uint8_t usart2_m_rx_buffer[SYS_RX_BUFFER_SIZE];

/* Buffers used for transmission */
static double_buffer_t usart2_m_tx_buffers;

/** Set uarte baudrate */
static bool usart2_set_baud(uint32_t baudrate);

/** Initialize DMA part */
static int usart2_init_dma(uint32_t baudrate);

/** Called each time a transfer is ready or when a previous transfer was finished. This function must be called with interrupts
 *  disabled or from interrupt context
 */
static int usart2_start_tx_lock(void);

int usart2_init(uint32_t baudrate, unsigned int flow_control)
{

    nrf_gpio_cfg_default(BOARD_USART2_TX_PIN);
    nrf_gpio_pin_set(BOARD_USART2_TX_PIN);
    nrf_gpio_cfg_default(BOARD_USART2_RX_PIN);
    nrf_gpio_pin_set(BOARD_USART2_RX_PIN);

    /* Ensue UART is disables before configuration*/
    NRF_UARTE1->ENABLE = UARTE_ENABLE_ENABLE_Disabled;

    /* Set the pins to be used for UART TX and RX*/
    NRF_UARTE1->PSEL.TXD = BOARD_USART2_TX_PIN;
    NRF_UARTE1->PSEL.RXD = BOARD_USART2_RX_PIN;
    NRF_UARTE1->TASKS_STOPTX = 1;
    NRF_UARTE1->TASKS_STOPRX = 1;

    /* Set the UART communication baudrate */
    if(!usart2_set_baud(baudrate)){
        return UART_OP_CONFIG_ERROR;
    }

    /* Initialize DMA p0rt*/
    if(usart2_init_dma(baudrate) != UART_OP_OK){
        return UART_OP_CONFIG_ERROR;
    }

    /* Configure TX pin and Enable UART*/
    nrf_gpio_cfg(BOARD_USART2_TX_PIN,
                         NRF_GPIO_PIN_DIR_OUTPUT,
                         NRF_GPIO_PIN_INPUT_CONNECT,
                         NRF_GPIO_PIN_NOPULL,
                         NRF_GPIO_PIN_S0S1,
                         NRF_GPIO_PIN_NOSENSE);

    NRF_UARTE1->ENABLE = UARTE_ENABLE_ENABLE_Enabled;

    /* Set rx parameters for the UART */
    if(usart2_set_rx() != UART_OP_OK){
        return UART_OP_CONFIG_ERROR;
    }

    return UART_OP_OK;
}

int usart2_set_rx(void){

    Sys_enterCriticalSection();

    /* Enable RX input */
    nrf_gpio_cfg(BOARD_USART2_RX_PIN,
                     NRF_GPIO_PIN_DIR_INPUT,
                     NRF_GPIO_PIN_INPUT_CONNECT,
                     NRF_GPIO_PIN_NOPULL,
                     NRF_GPIO_PIN_S0S1,
                     NRF_GPIO_PIN_SENSE_LOW);
    
    /* Clear all active events */
    NRF_UARTE1->EVENTS_RXDRDY = 0;
    NRF_UARTE1->EVENTS_ENDRX = 0;
    NRF_UARTE1->EVENTS_RXSTARTED = 0;

    // Prepare RX buffer
    NRF_UARTE1->RXD.PTR = (uint32_t) usart2_m_rx_buffer;
    NRF_UARTE1->RXD.MAXCNT = SYS_RX_BUFFER_SIZE;

    /*Start the Uart again */
    NRF_UARTE1->EVENTS_RXSTARTED = 0;
    NRF_UARTE1->TASKS_STARTRX = 1;

    // Confirm that the reception has been started again */
    while(NRF_UARTE1->EVENTS_RXSTARTED != 1){}


    Sys_exitCriticalSection();

    return UART_OP_OK;
}

int usart2_rx(uint8_t* data, unsigned int len){

    Sys_enterCriticalSection();

    /* Stop data reception */
    NRF_UARTE1->TASKS_STOPRX = 1;

    /* Wait for the reception to stop*/
    while(NRF_UARTE1->EVENTS_ENDRX != 1){}

    /**
     * For some reason the RXD.AMOUNT Register still returns zero at this point
     * @todo Find out why and use it to get the number of bytes received.
     */

    int bytes = (int) strlen((const char*) usart2_m_rx_buffer);

    if(bytes == 0 ){
        Sys_exitCriticalSection();
        return UART_OP_NO_DATA;
    }

    /* Copy the received bytes from the rx_buffer to user buffer */
    memcpy(data, usart2_m_rx_buffer, bytes);

    /*Start the Uart again */
    NRF_UARTE1->EVENTS_RXSTARTED = 0;
    NRF_UARTE1->TASKS_STARTRX = 1;

    /* Confirm that the reception has been started again */
    while(NRF_UARTE1->EVENTS_RXSTARTED != 1){}

    Sys_exitCriticalSection();

    return bytes;

}

int usart2_clear_buffer(void){

    Sys_enterCriticalSection();

    /* Stop RX */
    NRF_UARTE1->TASKS_STOPRX = 1;
    while(NRF_UARTE1->EVENTS_RXTO != 1){}

    /* Clear buffer*/
    memset(usart2_m_rx_buffer, 0, sizeof(usart2_m_rx_buffer));

    /* Start RX*/
    NRF_UARTE1->EVENTS_RXSTARTED = 0;
    NRF_UARTE1->TASKS_STARTRX = 1;

    /* Confirm that the reception has been started again */
    while(NRF_UARTE1->EVENTS_RXSTARTED != 1){}

    Sys_exitCriticalSection();

    return UART_OP_OK;
}

uint32_t usart2_tx(const void * buffer, uint32_t length)
{
    Sys_enterCriticalSection();

    if(usart2_clear_buffer() != UART_OP_OK){
        return 0;
    }

    // Check if there is enough room
    if (BUFFER_SIZE - DoubleBuffer_getIndex(usart2_m_tx_buffers) < length)
    {
        Sys_exitCriticalSection();
        return 0;
    }

    // Copy data to current buffer
    memcpy(DoubleBuffer_getActive(usart2_m_tx_buffers) +
           DoubleBuffer_getIndex(usart2_m_tx_buffers),
           buffer,
           length);

    DoubleBuffer_incrIndex(usart2_m_tx_buffers, length);

    int ret =  usart2_start_tx_lock();

    Sys_exitCriticalSection();

    return ret;
}

static int usart2_start_tx_lock(void)
{

    if (DoubleBuffer_getIndex(usart2_m_tx_buffers) == 0)
    {
        // Nothing to send
        return -1;
    }

    // Start DMA
    NRF_UARTE1->TXD.PTR = (uint32_t) DoubleBuffer_getActive(usart2_m_tx_buffers);
    NRF_UARTE1->TXD.MAXCNT = DoubleBuffer_getIndex(usart2_m_tx_buffers);

    NRF_UARTE1->EVENTS_ENDTX = 0;
    NRF_UARTE1->TASKS_STARTTX = 1;

    /* Wait for the transmission to complete */
    while(NRF_UARTE1->EVENTS_ENDTX == 0){}

    // Swipe buffers (it automatically reset writing index)
    DoubleBuffer_swipe(usart2_m_tx_buffers);

    /** Return number of bytes transmitted  */
    return NRF_UARTE1->TXD.AMOUNT;
}


static bool usart2_set_baud(uint32_t baudrate)
{
    bool ret = true;
    switch (baudrate)
    {
    case 115200:
        NRF_UARTE1->BAUDRATE = (uint32_t)UARTE_BAUDRATE_BAUDRATE_Baud115200;
        break;
    case 125000:
        // This value is not from official nrf9160_bitfields.h
        NRF_UARTE1->BAUDRATE = (uint32_t)(0x02000000UL);
        break;
    case 250000:
        NRF_UARTE1->BAUDRATE = (uint32_t)UARTE_BAUDRATE_BAUDRATE_Baud250000;
        break;
    case 460800:
        NRF_UARTE1->BAUDRATE = (uint32_t)UARTE_BAUDRATE_BAUDRATE_Baud460800;
        break;
    case 1000000:
        NRF_UARTE1->BAUDRATE = (uint32_t)UARTE_BAUDRATE_BAUDRATE_Baud1M;
        break;
    default:
        // Intended baudrate is not in the list, default baudrate from chip will be used
        ret = false;
        break;
    }

    return ret;
}

static int usart2_init_dma(uint32_t baudrate)
{

    NRF_UARTE1->INTENCLR = 0xffffffffUL;
    NRF_UARTE1->INTENSET =
        (UARTE_INTEN_ENDTX_Enabled << UARTE_INTEN_ENDTX_Pos) |
        (UARTE_INTEN_ERROR_Enabled << UARTE_INTEN_ERROR_Pos);

    NRF_UARTE0->SHORTS =
        UARTE_SHORTS_ENDRX_STARTRX_Enabled << UARTE_SHORTS_ENDRX_STARTRX_Pos;
 
    /* Configure TX part */
    DoubleBuffer_init(usart2_m_tx_buffers);

    if(DoubleBuffer_getIndex(usart2_m_tx_buffers) != 0){
        return UART_OP_CONFIG_ERROR;
    }

    return UART_OP_OK;
}
