#include <fsm.h>

#define DEBUG_LOG_MODULE_NAME "FSM"
#define DEBUG_LOG_MAX_LEVEL LVL_INFO
#include "debug_log.h"

void init_state_machine(StateMachine *sm, int initial_state, StateHandlerFunc* handlers) {
    sm->current_state = initial_state;
    sm->is_running = true;
    sm->state_handlers = handlers;  // Assign state handlers array
}

void run_state_machine(StateMachine *sm) {
    if (sm->is_running) {
        sm->current_state = base_state_transition(sm); // Perform state transition
        if (sm->current_state == -1) {
            sm->is_running = false; // Stop if in a terminal state
        }
    }
}

int base_state_transition(StateMachine *sm) {
    // Call the state handler for the current state
    int ret = sm->state_handlers[sm->current_state](sm);

    LOG(LVL_ERROR, "Transitioning from state: %d >> %d\n", sm->current_state, ret);


    return ret; // Call the current state handler
}
