#include "eSD.h"
#include "eWeb.h"


#define ESD_HANDLERS(concidional_funtion,handler_html,handler_statics) \
    {{"/sd.html", HTTP_GET , sd_get_handler , NULL}, true, {sd_html_asm_start,sd_html_asm_end,"",concidional_funtion}}



/* {{"/css/ota.css", HTTP_GET , handler_statics , NULL}, true, {ota_css_asm_start,ota_css_asm_end,"text/css",NULL}}, \
// {{"/js/ota.js", HTTP_GET , handler_statics , NULL}, true, {ota_js_asm_start,ota_js_asm_end,"text/javascript",NULL}}, \
 {{"/ota_update", HTTP_POST, concidional_funtion, NULL}, true, {.uri_handler_function = ota_post_handler}}, \
*/

extern const char sd_html_asm_start[] asm("_binary_sd_html_start");
extern const char sd_html_asm_end[] asm("_binary_sd_html_end");

esp_err_t sd_get_handler(httpd_req_t *req);