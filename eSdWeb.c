#include "eSdWeb.h"

#include <dirent.h>
#include "esp_log.h"

char ad[200];

void list_files() {
    // Abrir el directorio de la tarjeta SD
    DIR* dir = opendir("/sdcard");
    if (dir == NULL) {
        strcat(ad,"No se puede abrir el directorio SD");
        return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Ignorar '.' y '..'
        if (entry->d_type == DT_REG) {  // Si es un archivo regular
            strcat(ad,"Archivo encontrado: ");
            strcat(ad,entry->d_name);
            strcat(ad,"\n");
        }
    }

    closedir(dir);
}


esp_err_t esdweb_handler(httpd_req_t *req){
    //DO SOMETHING
    static_ctx_handler *ctx = (static_ctx_handler *)req->user_ctx;
    if ( ctx && ctx->uri_execution_function) {
        if(!ctx->uri_execution_function(req)){
            httpd_resp_set_status(req, "400 Bad Request");
            httpd_resp_set_hdr(req, "Content-Type", "text");
            httpd_resp_send(req, "No conditional function pass", HTTPD_RESP_USE_STRLEN);
            return ESP_OK;
        } 
    } 
    list_files();
    char*buffer;
    EWEB_GENERATE_REPLACEMENT_BUFFER(buffer,sdweb_html_asm_start,ad);
    httpd_resp_set_type(req, "text/html");
    esp_err_t err = eweb_send_resp_try_chunk(req,buffer,strlen(buffer));
    free(buffer);
    return err;
}