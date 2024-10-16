#ifndef _METER_STATE_MACHINE_H
#define _METER_STATE_MACHINE_H

#include <stdio.h>
#include <stdbool.h>
#include <enlight.h>

// Meter-specific states
typedef enum {
    METER_STATE_IDLE,
    METER_STATE_CONFIG,
    METER_STATE_ACTIVE,
    METER_STATE_ERROR,
    METER_STATE_MAX // Sentinel value for state count
} MeterState;

typedef struct MeterStateMachine MeterStateMachine;

// Function pointer type for state handler functions
typedef int (*StateHandlerFunc)(struct MeterStateMachine *);

// MeterStateMachine structure containing all necessary elements
struct MeterStateMachine {
    int current_state;             // Current state of the machine
    bool is_running;               // Flag to indicate if the machine is running

    StateHandlerFunc* state_handlers; // Pointer to an array of state handler functions

    meter_t enlight_meter;          // Embedded meter object
};

// Meter state handler functions
int meter_idle_state(MeterStateMachine *msm);
int meter_config_state(MeterStateMachine *msm);
int meter_active_state(MeterStateMachine *msm);
int meter_error_state(MeterStateMachine *msm);

// Initialize and run the meter state machine
void init_meter_state_machine(MeterStateMachine *msm, int initial_state);
void run_meter_state_machine(MeterStateMachine *msm);

#endif // _METER_STATE_MACHINE_H
