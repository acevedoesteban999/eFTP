#pragma once
#include "eSD.h"
#include "eWeb.h"


#define ESDWEB_HANDLERS( execution_funtion ,handler_html,handler_statics) \
    {{"/sdweb.min.html", HTTP_GET , handler_html , NULL}, true, {sdweb_min_html_asm_start,sdweb_min_html_asm_end,"",NULL,NULL}} \    


// {{"/js/sdweb.min.js", HTTP_GET , handler_statics , NULL}, true, {sdweb_min_js_asm_start,sdweb_min_js_asm_end,"",NULL,NULL}} 
// {{"/sdweb_get_data", HTTP_POST , execution_funtion , NULL}, true, {.uri_execution_function = esdweb_get_data_post_handler}} 
    


typedef struct
{
    char filename[256];
    uint32_t size;
}esdweb_data;

extern const char sdweb_min_html_asm_start[] asm("_binary_sdweb_min_html_start");
extern const char sdweb_min_html_asm_end[] asm("_binary_sdweb_min_html_end");

extern const char sdweb_min_js_asm_start[] asm("_binary_sdweb_min_js_start");
extern const char sdweb_min_js_asm_end[] asm("_binary_sdweb_min_js_end");


esp_err_t esdweb_get_data_post_handler(httpd_req_t *req);