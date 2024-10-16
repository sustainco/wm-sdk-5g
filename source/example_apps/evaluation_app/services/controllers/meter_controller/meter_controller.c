#include "meter_controller.h"
#include <token_injection_response_queue.h>
#include <token_recv_queue.h>

#define DEBUG_LOG_MODULE_NAME "METER_FSM"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO
#include "debug_log.h"

// generic_queue_t token_injection_response_queue;
// generic_queue_t token_rcv_queue;

// Meter state handlers array
StateHandlerFunc meter_state_handlers[METER_STATE_MAX] = {
    meter_idle_state,
    meter_config_state,
    meter_active_state,
    meter_error_state
};

// Initialize the meter state machine
void init_meter_state_machine(MeterStateMachine *msm, int initial_state) {
    msm->current_state = initial_state;
    msm->is_running = true;
    msm->state_handlers = meter_state_handlers; // Assign meter-specific state handlers

    // Create an enlight meter object
    meter_create(&msm->enlight_meter);
}

// Run the meter state machine
void run_meter_state_machine(MeterStateMachine *msm) {
    if (msm->is_running) {
        int next_state = msm->state_handlers[msm->current_state](msm); // Call current state handler
        LOG(LVL_DEBUG, "Transitioning from state: %d >> %d\n", msm->current_state, next_state);
        if (next_state == -1) {
            msm->is_running = false; // Stop if in a terminal state
        } else {
            msm->current_state = next_state;
        }
    }
}

// Meter state handler functions
int meter_idle_state(MeterStateMachine *msm) {

    for(volatile int i = 0; i < 2000000; i++){}
    
    int ret = msm->enlight_meter.init(&msm->enlight_meter);
    if (ret == METER_OP_OK) {
        return METER_STATE_CONFIG;
    }
    return METER_STATE_IDLE; 
}

int meter_config_state(MeterStateMachine *msm) {
    int ret = msm->enlight_meter.configure(&msm->enlight_meter);
    if (ret != METER_OP_OK) {
        return METER_STATE_ERROR;
    }
    return METER_STATE_ACTIVE; 
}

int meter_active_state(MeterStateMachine *msm) {
    int ret = msm->enlight_meter.update_realtime_data(&msm->enlight_meter);
    if (ret != METER_OP_OK) {
        return METER_STATE_ERROR;
    }

    if((!queue_is_empty(&token_rcv_queue))){
        token_t rcv_token = {
            .serial = 0,
        };

        queue_pop(&token_rcv_queue, &rcv_token);

        if(rcv_token.serial != 0){

            token_injection_ctx ctx;
            ctx.serial = rcv_token.serial;
            strncpy(ctx.token, rcv_token.token, 20);

            int ret = msm->enlight_meter.token_injection_tx(&msm->enlight_meter, &ctx);

            if(ret != METER_OP_OK){
                LOG(LVL_ERROR, "Error Injecting Token\n");
            }

            token_injection_response_t injection_resp = {
                .start_x = 0x70ffee,
                .serial = ctx.serial,
                .injected_units = ctx.injected_tokens,
                .is_valid = ret,
                .end_x = 0x90ffee,
            };

            if(!queue_is_full(&token_injection_response_queue)){
                queue_push(&token_injection_response_queue, &injection_resp);
            }

            if(ret != METER_OP_OK){
                return METER_STATE_ERROR;
            }

        }

    }

    return METER_STATE_ACTIVE; 
}

int meter_error_state(MeterStateMachine *msm) {
    LOG(LVL_ERROR, "Error communicating with Meter\n");
    return METER_STATE_CONFIG; 
}
