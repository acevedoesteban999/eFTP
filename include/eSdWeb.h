#pragma once
#include "eSD.h"
#include "eWeb.h"


#define ESDWEB_HANDLERS( execution_funtion ,handler_html,handler_statics) \
    {{"/sd.html", HTTP_GET , execution_funtion , NULL}, true, {sdweb_html_asm_start,sdweb_html_asm_end,"",esdweb_handler,NULL}}



/* {{"/css/ota.css", HTTP_GET , handler_statics , NULL}, true, {ota_css_asm_start,ota_css_asm_end,"text/css",NULL}}, \
// {{"/js/ota.js", HTTP_GET , handler_statics , NULL}, true, {ota_js_asm_start,ota_js_asm_end,"text/javascript",NULL}}, \
 {{"/ota_update", HTTP_POST, concidional_funtion, NULL}, true, {.uri_execution_function = ota_post_handler}}, \
*/

extern const char sdweb_html_asm_start[] asm("_binary_sdweb_html_start");
extern const char sdweb_html_asm_end[] asm("_binary_sdweb_html_end");

esp_err_t esdweb_handler(httpd_req_t *req);