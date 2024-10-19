#ifndef _METER_DATA_H
#define _METER_DATA_H

#include <stdint.h>

typedef struct meter_data {

    int start_x;

    int device_type; //4
    int producer; //4
    int billing_type;//4

    uint64_t serial;

    int supply_group;//4
    int meter_status;
    int tamper_status;
    int power_limit;

    int kwh_consumed;
    int kwh_forward;
    int kwh_reverse;
    int peak_demand;
    int available_credit;
    int time_stamp;

    int end_x;

}meter_data_t;

meter_data_t* get_meter_data_instance();





#endif //_METER_DATA_H