#ifndef _TOKEN_INJECTION_RESPONSE_H
#define _TOKEN_INJECTION_REsPONSE_H

#include <stdint.h>



typedef struct token_injection_response{

    int start_x;

    uint64_t serial;

    int injected_units;
    int operation_status;

    int is_valid;

    int end_x;

}token_injection_response_t;


token_injection_response_t* get_token_response_data_instance();



#endif //_TOKEN_INJECTION_RESPONSE_H
