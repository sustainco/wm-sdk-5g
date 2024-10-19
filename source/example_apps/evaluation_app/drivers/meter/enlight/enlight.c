#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <enlight.h>
#include <command_builder.h>
#include <meter_hex_to_int.h>
#include <board.h>

#include <delay_timer.h>

// #include "payload_parser.h"
#include "meter_data.h"

#define DEBUG_LOG_MODULE_NAME "ENLIGHT M"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO
#include "debug_log.h"

#define REALTIME_READ_REGISTERS_NUM 2

/**
 * @brief List of commands to get information from meter
 */
uint8_t meter_info_commands[5][10] = {
    {0x01, 0x52, 0x02, 0x32, 0x30, 0x30, 0x36, 0x30, 0x03, 0x67}, // Meter Serial Number
    {0x01, 0x52, 0x02, 0x32, 0x30, 0x30, 0x32, 0x30, 0x03, 0x63}, // Meter status
    {0x01, 0x52, 0x02, 0x32, 0x30, 0x31, 0x34, 0x30, 0x03, 0x64}, // Tamper Status
    {0x01, 0x52, 0x02, 0x32, 0x30, 0x31, 0x36, 0x30, 0x03, 0x66}, // Supply Group Code
    {0x01, 0x52, 0x02, 0x32, 0x30, 0x30, 0x43, 0x30, 0x03, 0x12}, // Maximum Power Limit
};

/**
 * @brief Address of registers to be read from the meter
 */
const uint16_t realtime_read_registers[REALTIME_READ_REGISTERS_NUM] = {
    0x2011, /* CumulativeEnergyConsumption */
    0x2010  /* AvailableElectricityCredit */
};

const uint16_t METER_TOKEN_ADDRESS = 0xFFFF;


/* Private method prototypes */
static int energy_meter_write(meter_t* self, char* command, uint8_t len);
static int energy_meter_read(meter_t* self, char* buffer, uint8_t len);
static int energy_meter_read_meter_address(meter_t* self, uint16_t address, char* rx_buffer, uint8_t size);
static int energy_meter_write_meter_address(meter_t* self, uint16_t address, char* data, char* rx_buffer, uint8_t size);
static int energy_meter_write_token(meter_t* self, char* token);

/* utilities */
static void energy_meter_encode_data(uint8_t* buffer, int len);
static void energy_meter_decode_data(uint8_t* buffer, int len);
static uint8_t energy_meter_calculate_parity(uint8_t data);

static int energy_meter_init(meter_t* self) {

    uart_create(&self->meter_interface);

    uart_cfg_t cfg = {
        .baudrate = 2400,
        .uart = NRF_UARTE1,
        .rx_pin = BOARD_USART2_RX_PIN,
        .tx_pin = BOARD_USART2_TX_PIN,
        .flow_control = UART2_FLOW_CONTROL_NONE,
    };

    if(self->meter_interface.init(&self->meter_interface, cfg) != UART_OP_OK){
        LOG(LVL_ERROR, "Error initializing meter UART");
        return METER_OP_CONFIG_ERROR;
    }

    /** Initialize the descriptive info about the meter */
    get_meter_data_instance()->device_type = 0x01;
    get_meter_data_instance()->producer = 0x01;
    get_meter_data_instance()->billing_type = 0x01;

    get_meter_data_instance()->start_x = 0xc0ffeeb0;
    get_meter_data_instance()->end_x = 0xd0ffeeb0;

    get_meter_data_instance()->serial = 0x09000030000;
    // get_meter_data_instance()->meter_status = 0x0F;
    // get_meter_data_instance()->supply_group = 0x00;
    // get_meter_data_instance()->tamper_status = 0x00;
    // get_meter_data_instance()->power_limit = 0x47e0;

    // get_meter_data_instance()->kwh_consumed = 50;
    // get_meter_data_instance()->available_credit = 29;
    // get_meter_data_instance()->kwh_forward = 0;
    // get_meter_data_instance()->kwh_reverse = 0;
    // get_meter_data_instance()->peak_demand = 0;
    // get_meter_data_instance()->time_stamp = 0;

    return METER_OP_OK; // Success
}

static int energy_meter_fetch_sensor_data(meter_t* self, meter_data_req_t req) {
    
    int ret = energy_meter_write(self, req.command, req.command_len);

	if (ret != req.command_len){
		LOG(LVL_ERROR, "Command not sent\n");
		return METER_OP_ERROR;
	}
	

    for(volatile uint64_t i = 0; i < 2000000 ; i++){};
  


	ret = energy_meter_read(self, req.buffer, req.buffer_size);

	if(ret <=  0){
        LOG(LVL_ERROR, "No response from meter, check connection\n");
		return METER_OP_ERROR;
	}

	/* null terminate the buffer */
	req.buffer[ret] = '\0';

    return ret;
}

static int energy_meter_configure(meter_t* self) {

    char recv_buffer[5][ENERGY_METER_RX_BUFFER_SIZE];
	
	for(int i = 0; i < 5; i++){


		meter_data_req_t req = {
			.command 		= (char*) meter_info_commands[i],
			.command_len 	= sizeof(meter_info_commands[i]),
			.buffer 		= recv_buffer[i],
			.buffer_size 	= ENERGY_METER_RX_BUFFER_SIZE
		};

		int ret = energy_meter_fetch_sensor_data(self, req);

		if( ret <= 0){
			return METER_OP_ERROR;
		}
	}

    get_meter_data_instance()->serial = extract_hex_value(recv_buffer[0]);
    get_meter_data_instance()->meter_status = extract_hex_value(recv_buffer[1]);
    get_meter_data_instance()->tamper_status = extract_hex_value(recv_buffer[2]);
    get_meter_data_instance()->supply_group = extract_hex_value(recv_buffer[3]);
    get_meter_data_instance()->power_limit = extract_hex_value(recv_buffer[4]);

	return METER_OP_OK;

}

static int energy_meter_update_realtime_data(meter_t* self) {
   /** Read the provided registers and update json payload */
	char data_receive_buffer[2][ENERGY_METER_RX_BUFFER_SIZE];

	for(int i = 0; i < REALTIME_READ_REGISTERS_NUM; i++){

		if(energy_meter_read_meter_address(self, realtime_read_registers[i], data_receive_buffer[i], sizeof(data_receive_buffer[i])) != METER_OP_OK){
			return METER_OP_ERROR;
		}
		
	}

    get_meter_data_instance()->kwh_consumed = extract_hex_value(data_receive_buffer[0]) * 0.1;
    get_meter_data_instance()->available_credit = extract_hex_value(data_receive_buffer[1]) * 0.1;
    get_meter_data_instance()->kwh_forward = 0;
    get_meter_data_instance()->kwh_reverse = 0;
    get_meter_data_instance()->peak_demand = 0;
	
	return METER_OP_OK;
}

static int energy_meter_token_injection_tx(meter_t* self, token_injection_ctx* ctx) {

    ctx->timestamp = 0;

    /* Validate the meter UID sent with the token */
    if( ctx->serial != get_meter_data_instance()->serial){
        ctx->tx_status = METER_TOKEN_INJECTION_FAIL;
        return METER_OP_ERROR;
    }

    /* Read the current meter available credits */
    if (energy_meter_update_realtime_data(self) != METER_OP_OK) {
        ctx->tx_status = METER_TOKEN_INJECTION_FAIL;
        return METER_OP_ERROR;
    }

    ctx->units_before_injection = get_meter_data_instance()->available_credit;

    /* Inject the received tokens */
    if (energy_meter_write_token(self, ctx->token) != METER_RESPONSE_ACK) {
        ctx->tx_status = METER_TOKEN_INJECTION_FAIL;
        return METER_OP_ERROR;
    }

    /* Read the available credits after injection */
    if (energy_meter_update_realtime_data(self) != METER_OP_OK) {
        ctx->tx_status = METER_TOKEN_INJECTION_FAIL;
        return METER_OP_ERROR;
    }

    ctx->units_after_injection = get_meter_data_instance()->available_credit;

    /* Calculate the token value */
    ctx->injected_tokens = ctx->units_after_injection - ctx->units_before_injection;

    if (ctx->injected_tokens <= 0) {
        ctx->tx_status = METER_TOKEN_INJECTION_FAIL;
        return METER_OP_ERROR;
    }

    /* Set tx status */
    ctx->tx_status = METER_TOKEN_INJECTION_SUCCESS;

    return METER_OP_OK;
}

#ifdef SIMULATE_METER_DATA
static int energy_meter_generate_realtime_data(meter_t* self, simulated_realtime_data_t* data) {
    // Simulate real-time meter data
    data->available_credits = 100; // Example
    data->kwh_consumed = 50; // Example
    return 0; // Success
}
#endif

/* Private method implementations */
static int energy_meter_write(meter_t* self, char* command, uint8_t len) {

    energy_meter_encode_data((uint8_t*) command, len);

    int ret = self->meter_interface.tx(&self->meter_interface, command, len);

    if(ret != len){
        return METER_OP_ERROR;
    }
    
    return ret;
}

static int energy_meter_read(meter_t* self, char* buffer, uint8_t len) {

    int ret = self->meter_interface.rx(&self->meter_interface, (uint8_t*)buffer, len);
    
    if(ret < 0){
        return METER_OP_ERROR;
    }

    energy_meter_decode_data((uint8_t*)buffer, ret);

    return ret; 
}

static int energy_meter_read_meter_address(meter_t* self, uint16_t address, char* rx_buffer, uint8_t size) {
   
    CommandResult cmd  = BuildReadCommand(address, 1);

	if(cmd.length <= 0){
		LOG(LVL_ERROR, "Error getting meter read command\n");
		return METER_OP_ERROR;
	}

	/** Clear receive buffer */
	memset(rx_buffer, 0, size);

	meter_data_req_t req = {
		.command 		= (char*) cmd.command,
		.command_len 	= (uint8_t) cmd.length,
		.buffer 		= rx_buffer,
		.buffer_size 	= size
	};

	int ret = energy_meter_fetch_sensor_data(self,req);

	if(ret <= 0){
		return METER_OP_ERROR;
	}

    return METER_OP_OK; // Success
}

static void energy_meter_encode_data(uint8_t* buffer, int len){

    for (int i = 0; i < len; i++) {
      buffer[i] = energy_meter_calculate_parity(buffer[i]);
    }

}

static void energy_meter_decode_data(uint8_t* buffer, int len){

    for(int i = 0; i < len; i++){
        buffer[i] = buffer[i] & 0x7F;
    }

}

static uint8_t energy_meter_calculate_parity(uint8_t data) {
    // Calculate parity for the given data byte
    int count = 0;
    for (int i = 0; i < 7; i++) {
        if (data & (1 << i)) {
            count++;
        }
    }
    // Calculate parity (even parity)
    int parity_bit = count % 2 == 0 ? 0 : 1;

    // Set the 8th bit (parity bit) in the data byte
    return (data & 0x7F) | (parity_bit << 7);
}

static int energy_meter_write_meter_address(meter_t* self, uint16_t address, char* data, char* rx_buffer, uint8_t size) {
    
    CommandResult cmd= BuildWriteCommand(address, data);

	if(cmd.length <= 0){
		LOG(LVL_ERROR, "Error getting command to write at address %04X \n", address);
		return -1;
	}

	memset(rx_buffer, 0, size);

	meter_data_req_t req = {
		.command 		= (char*) cmd.command,
		.command_len 	= (uint8_t) cmd.length,
		.buffer 		= rx_buffer,
		.buffer_size 	= size
	};

	int ret = energy_meter_fetch_sensor_data(self, req);

	if(ret <= 0){
		return METER_OP_ERROR;
	}

	return ret;
}

static void remove_spaces(char* str) {
    int i = 0, j = 0;
    while (str[i]) {
        if (str[i] != ' ') {
            str[j++] = str[i];
        }
        i++;
    }
    str[j] = '\0';  // Null-terminate the resulting string
}

static int energy_meter_write_token(meter_t* self, char* token) {
    
    /* Remove spaces from the token */
    remove_spaces(token);

	char rx_response[ENERGY_METER_RX_BUFFER_SIZE];

	int ret = energy_meter_write_meter_address(self, METER_TOKEN_ADDRESS, token, rx_response, sizeof(rx_response));

	if (ret <= 0){
		return METER_RESPONSE_NAN;
	}

	if(rx_response[ret-1] == METER_RESPONSE_ACK ){
		return METER_RESPONSE_ACK;
	}
	else if(rx_response[ret-1] == METER_RESPONSE_NAK){
		return METER_RESPONSE_NAK;
	}

	return METER_RESPONSE_NAN;
}

/* Function to get the instance of the meter */
void meter_create(meter_t *self){

    self->init = energy_meter_init;
    self->configure = energy_meter_configure;
    self->update_realtime_data = energy_meter_update_realtime_data;
    self->token_injection_tx = energy_meter_token_injection_tx,
    self->fetch_sensor_data = energy_meter_fetch_sensor_data;

}
