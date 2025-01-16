#include "eSdWeb.h"


// void list_files() {
//     // Abrir el directorio de la tarjeta SD
//     DIR* dir = opendir("/sdcard");
//     if (dir == NULL) {
//         ESP_LOGE(TAG, "No se puede abrir el directorio SD");
//         return;
//     }

//     struct dirent* entry;
//     while ((entry = readdir(dir)) != NULL) {
//         // Ignorar '.' y '..'
//         if (entry->d_type == DT_REG) {  // Si es un archivo regular
//             ESP_LOGI(TAG, "Archivo encontrado: %s", entry->d_name);
//         }
//     }

//     closedir(dir);
// }


esp_err_t esdweb_handler(httpd_req_t *req){
    //DO SOMETHING
    static_ctx_handler *ctx = (static_ctx_handler *)req->user_ctx;
    if ( ctx && ctx->uri_handler_function) {
        if(!ctx->uri_handler_function(req)){
            httpd_resp_set_status(req, "400 Bad Request");
            httpd_resp_set_hdr(req, "Content-Type", "text");
            httpd_resp_send(req, "No conditional function pass", HTTPD_RESP_USE_STRLEN);
            return ESP_OK;
        } 
    } 
    
    httpd_resp_set_type(req, "text/html");
    return eweb_send_resp_try_chunk(req,sdweb_html_asm_start,sdweb_html_asm_end - sdweb_html_asm_start);
}