#ifndef _FSM_H
#define _FSM_H

#include <stdio.h>
#include <stdbool.h>

typedef struct StateMachine StateMachine;
typedef int (*StateHandlerFunc)(struct StateMachine *);

// Base StateMachine "class"
struct StateMachine {

    int current_state;             // Current state of the machine
    bool is_running;               // Flag to indicate if the machine is running

    // State transition function pointer
    int (*state_transition)(struct StateMachine *);

    // Pointer to an array of function pointers for state handlers
    StateHandlerFunc* state_handlers;

};


// Initialize the base state machine
void init_state_machine(StateMachine *sm, int initial_state, StateHandlerFunc* handlers);

// Run the base state machine (ticks through states)
void run_state_machine(StateMachine *sm);

// Base state transition function
int base_state_transition(StateMachine *sm);


#endif //_FSM_H