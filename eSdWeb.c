#include "eSdWeb.h"

#include <dirent.h>
#include "ff.h"
#include "esp_log.h"

#define MAX_FILES 20


esp_err_t esdweb_get_data_post_handler(httpd_req_t *req) {
    char *buff_request;
    EWEB_ALOCATE_GET_ALL_DATA_REQUEST(req, buff_request);
    
    int dataI;
    eweb_get_int_urlencoded(buff_request, "index", &dataI);
    
    DIR* dir = opendir(ESD_MOUNT_POINT);
    if (dir == NULL) {
        ESP_LOGW("", "No se puede abrir el directorio SD\n");
        return ESP_FAIL;
    }

    esdweb_data* data = malloc(MAX_FILES * sizeof(esdweb_data));
    if (data == NULL) {
        ESP_LOGW("", "No se pudo asignar memoria para data\n");
        closedir(dir);
        return ESP_FAIL;
    }

    int buffer_counter = 0;
    int file_counter = 0;
    struct dirent* entry;
    int buffer_size = 0;
    FILINFO file_info;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            
            if (file_counter++ < dataI * MAX_FILES)
                continue;
            
            
            FRESULT f = f_stat(entry->d_name, &file_info);
            
            if(f == FR_OK){
                strncpy(data[buffer_counter].filename, entry->d_name, sizeof(data[buffer_counter].filename));
                buffer_size += strlen(entry->d_name);

                data[buffer_counter++].size = file_info.fsize;
                
                if (buffer_counter >= MAX_FILES)
                    break;
            }
        }
    }

    closedir(dir);

    buffer_size += 4*buffer_counter;

    char* buff = calloc(buffer_size + 1, sizeof(char));
    if (buff == NULL) {
        httpd_resp_send_err((req), HTTPD_500_INTERNAL_SERVER_ERROR, "No Memory for allocate");
        free(buff_request);
        return ESP_FAIL;
    }

    char input[260];
    for (unsigned i = 0; i < buffer_counter; i++) {
        if (i != 0)
            snprintf(input, sizeof(input), "&fn=%s", data[i].filename); 
        else
            snprintf(input, sizeof(input), "fn=%s", data[i].filename); 

        strcat(buff,input);
    }

    httpd_resp_set_type(req, "application/x-www-form-urlencoded");
    eweb_send_resp_try_chunk(req, buff,strlen(buff) );
    free(buff);
    free(buff_request);

    return ESP_OK;
}


// esp_err_t list_files_on_sdcard(httpd_req_t *req) {
    
//     char *buff_request;
//     EWEB_ALOCATE_GET_ALL_DATA_REQUEST(req,buff_request);
//     int dataI;
//     eweb_get_int_urlencoded(buff_request,"index",&dataI);
    
//     DIR* dir = opendir(ESD_MOUNT_POINT);
//     if (dir == NULL) {
//         ESP_LOGW("", "No se puede abrir el directorio SD\n");
//         return ESP_FAIL;
//     }

//     esdweb_data data[MAX_FILES];
//     char filepath[512];
//     int buffer_counter = 0;
//     int file_counter = 0;
//     struct dirent* entry;

//     FRESULT fr;
//     FILINFO file_info;

//     while ((entry = readdir(dir)) != NULL) {
//         if (entry->d_type == DT_REG) { 
//             if(file_counter++ < dataI * MAX_FILES)
//                 continue;
            
//             snprintf(filepath, sizeof(filepath), "%s/%s", ESD_MOUNT_POINT, entry->d_name);

//             fr  = f_stat(filepath, &file_info);
//             strncpy(data[buffer_counter].filename, entry->d_name, sizeof(data[buffer_counter].filename));
//             data[buffer_counter].type = file_info.fsize;
//             if (buffer_counter++ >= MAX_FILES)
//                 break;
//         }
//     }

//     closedir(dir);
//     char*buff = calloc(sizeof(char),MAX_FILES * 255 + 1);
//         if (buff == NULL) {
//             httpd_resp_send_err((req), HTTPD_500_INTERNAL_SERVER_ERROR, "No Memory for alocate");
//             return ESP_FAIL;
//         }
//     char input[260];
//     for(unsigned i =0 ; i <buffer_counter;i++){
//         if(i!=0)
//             strcat(input,"&fn=%s");
//         else
//             strcat(input,"fn=%s");
//         snprintf(buff,sizeof(buff),input,data[i].filename);
//     }
    
    
//     free(buff_request);
//     eweb_send_resp_try_chunk(req,buff,strlen(buff));
//     free(buff);

//     return ESP_OK;
// }