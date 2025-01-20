#include "eFTP.h"

#include <dirent.h>
#include "ff.h"
#include "esp_log.h"

#define MAX_FILES 20
#define FILE_CHUNK_SIZE 5120

esp_err_t eftp_get_data_post_handler(httpd_req_t *req) {
    char *buff_request;
    EWEB_ALOCATE_GET_ALL_DATA_REQUEST(req, buff_request);
    
    int dataI;
    eweb_get_int_urlencoded(buff_request, "index", &dataI);
    
    DIR* dir = opendir(ESD_MOUNT_POINT);
    if (dir == NULL) {
        ESP_LOGW("", "No se puede abrir el directorio SD\n");
        return ESP_FAIL;
    }

    eftp_data* data = malloc(MAX_FILES * sizeof(eftp_data));
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

esp_err_t eftp_get_file_post_handler(httpd_req_t *req){
    char *buff_request;
    char filename[256];
    char path[256 + strlen(ESD_MOUNT_POINT) + 2];
    EWEB_ALOCATE_GET_ALL_DATA_REQUEST(req, buff_request);
    EWEB_CHECK_PARAMETER_STR_URLENCODED(req,buff_request,"filename",filename,sizeof(filename));
    strcpy(path,ESD_MOUNT_POINT);
    strcat(path,"/");
    strcat(path,filename);
    FILE *file = NULL;
    char *chunk = NULL;
    error_t err = ESP_OK;
    file = fopen(path, "rb");
    if (!file) {
        ESP_LOGE("","File not found: %s", path);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
        return ESP_FAIL;
    }
    chunk = (char *)malloc(FILE_CHUNK_SIZE);
    if (!chunk) {
        ESP_LOGE("", "Failed to allocate memory for file chunk");
        
        fclose(file);
        return ESP_FAIL;
    }
    httpd_resp_set_type(req, "application/octet-stream");
    
    size_t read_bytes;
    while ((read_bytes = fread(chunk, 1, FILE_CHUNK_SIZE, file)) > 0) {
        if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK) {
            ESP_LOGE("FILE_HANDLER", "Error sending chunk");
            err = ESP_FAIL;
            break;
        }
    }
    if (ferror(file)) {
        ESP_LOGE("FILE_HANDLER", "Error reading the file");
        err = ESP_FAIL;
    } 

    httpd_resp_send_chunk(req, NULL, 0);
    if(err != ESP_OK)
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal Server Error");
    free(chunk);
    fclose(file);
    
    return err;
}