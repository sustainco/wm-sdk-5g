#include <meter_data.h>

static meter_data_t default_meter_data;

meter_data_t* get_meter_data_instance(){
    return &default_meter_data;
}