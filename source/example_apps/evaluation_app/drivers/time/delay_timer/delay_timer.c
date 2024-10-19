#include <delay_timer.h>

static void delay_timer_reset(delay_timer_t* self){
    self->ms_counter = 0;
}

static uint64_t delay_timer_get(delay_timer_t* self){
    return self->ms_counter;
}

static void delay_timer_tick(delay_timer_t* self){
    self->ms_counter += 50;
}

 

delay_timer_t* get_delay_timer_instance(){

   static delay_timer_t _delay_timer = {
    .get = delay_timer_get,
    .reset = delay_timer_reset,
    .tick = delay_timer_tick,
   };

    return &_delay_timer;
}

