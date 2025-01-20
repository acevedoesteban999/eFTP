#pragma once
#include "eSD.h"
#include "eWeb.h"


#define EFTP_HANDLERS( execution_funtion ,handler_html,handler_statics) \
    {{"/ftp.min.html", HTTP_GET , handler_html , NULL}, true, {ftp_min_html_asm_start,ftp_min_html_asm_end,"",NULL,NULL}}, \
    {{"/js/ftp.min.js", HTTP_GET , handler_statics , NULL}, true, {ftp_min_js_asm_start,ftp_min_js_asm_end,"",NULL,NULL}}, \
    {{"/ftp_get_data", HTTP_POST , execution_funtion , NULL}, true, {.uri_execution_function = eftp_get_data_post_handler}}, \
    {{"/ftp_get_file", HTTP_POST , execution_funtion , NULL}, true, {.uri_execution_function = eftp_get_file_post_handler}} \
    


typedef struct
{
    char filename[256];
    uint32_t size;
}eftp_data;

extern const char ftp_min_html_asm_start[] asm("_binary_ftp_min_html_start");
extern const char ftp_min_html_asm_end[] asm("_binary_ftp_min_html_end");

extern const char ftp_min_js_asm_start[] asm("_binary_ftp_min_js_start");
extern const char ftp_min_js_asm_end[] asm("_binary_ftp_min_js_end");

esp_err_t eftp_get_data_post_handler(httpd_req_t *req);
esp_err_t eftp_get_file_post_handler(httpd_req_t *req);
