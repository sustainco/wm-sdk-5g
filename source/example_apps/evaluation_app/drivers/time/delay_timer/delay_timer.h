#ifndef _DELAY_TIMER_H
#define _DELAY_TIMER_H

#include <stdint.h>

typedef struct delay_timer_t delay_timer_t;

struct delay_timer_t{ 

    volatile uint64_t ms_counter;

    void (*reset)(struct delay_timer_t*);

    uint64_t (*get)(struct delay_timer_t*);

    void (*tick)(struct delay_timer_t*);

};


delay_timer_t* get_delay_timer_instance();

#endif //_DELAY_TIMER_H