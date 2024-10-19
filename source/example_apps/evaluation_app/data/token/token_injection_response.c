#include <token_injection_response.h>

static token_injection_response_t default_response;

token_injection_response_t* get_token_response_data_instance(){
    return &default_response;
}