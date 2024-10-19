#ifndef _ENLIGHT_METER_H
#define _ENLIGHT_METER_H

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <usart2.h>
#include <meter_data.h>

#define ENERGY_METER_RX_BUFFER_SIZE 100

/**
 * @brief Struct used to by the data fetcher to obtain data.
 * 
 */
typedef struct meter_data_req {
    char* command;      /** Command to send to the meter */
    uint8_t command_len; /* Length of the command */
    char* buffer;       /* Buffer to store the response data */
    uint8_t buffer_size; /* Size of the buffer */
} meter_data_req_t;

/**
 * @brief Response from the meter after an operation
 * 
 */
typedef enum meter_response {
    METER_RESPONSE_ACK = 0x06,
    METER_RESPONSE_NAK = 0x15,
    METER_RESPONSE_NAN = 0xFF
} meter_response_t;

typedef enum {
    METER_OP_ERROR,
    METER_OP_CONFIG_ERROR,
    METER_OP_OK
}meter_op_t;


#ifdef SIMULATE_METER_DATA
/**
 * @brief Struct to store simulated realtime data from the meter
 * 
 */
typedef struct simulated_realtime_data {
    int available_credits;
    int kwh_consumed;
} simulated_realtime_data_t;
#endif 

/**
 * @brief Current context of a token injection
 * 
 */
typedef struct {
    uint32_t timestamp;
    uint64_t serial;
    char token[50];
    int tx_status;
    float units_before_injection;
    float units_after_injection;
    float injected_tokens;
} token_injection_ctx;

/**
 * @brief Token injection operation status
 * 
 */
typedef enum {
    METER_TOKEN_INJECTION_SUCCESS = 0x01,
    METER_TOKEN_INJECTION_FAIL = 0x00
}token_injection_status_e;


/**
 * @brief meter class representation
 * 
 */
typedef struct meter {

    /* meter data */
    meter_data_t m_data;

    /* Meter comminication interface */
    uart_t meter_interface;

    /* Methods */
    /**
     * @brief Initialize the meter
     * @param self meter object
     * @return int operation status
     */
    int (*init)(struct meter* self);

    /**
     * @brief Configure the meter by reading the configuration data from the meter
     * @param self meter object
     * @return int operation status
     * 
     */
    int (*configure)(struct meter* self);

    /**
     * @brief get realtime data from the meter and update the data object
     * @param self meter object
     * @return int operation status
     * 
     */
    int (*update_realtime_data)(struct meter* self);

    /**
     * @brief Transaction to inject tokens into the meter
     * @param self meter_object
     * @param ctx current context information
     * @return int operation status
     * 
     */
    int (*token_injection_tx)(struct meter* self, token_injection_ctx* ctx);

    /**
     * @brief fetch data from the sensor. takes in a request and populates th requests response
     * @param self meter object
     * @param req meter data request object (meter_data_req_t)
     * @return int operation status
     */
    int (*fetch_sensor_data)(struct meter* self, meter_data_req_t req);

} meter_t;


/* Create a meter */
void meter_create(meter_t *self);



#endif //_ENLIGHT_METER_H