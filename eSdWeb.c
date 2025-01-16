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


esp_err_t sd_get_handler(httpd_req_t *req){
    //DO SOMETHING
    eweb_call_condicional_function(req);
}