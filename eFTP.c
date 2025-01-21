#include "eFTP.h"

#include <dirent.h>
#include "ff.h"
#include "esp_log.h"
#include "diskio.h"

#define MAX_FILES 20
#define FILE_CHUNK_SIZE 5120

esp_err_t eftp_get_data_post_handler(httpd_req_t *req) {
    FATFS *fs;
    DWORD fre_clust, fre_sect, tot_sect;
    uint64_t total_bytes = 0,used_bytes = 0;
    
    if(esd_get_error()){
        if (f_mount(fs, ESD_MOUNT_POINT, 1) != FR_OK) {
            ESP_LOGE("", "Error at mount SD");
            httpd_resp_send_err((req), HTTPD_500_INTERNAL_SERVER_ERROR, "Error at mount SD");
            return ESP_FAIL;
        }
    }

    if (f_getfree(ESD_MOUNT_POINT, &fre_clust, &fs) == FR_OK) {
        tot_sect = (fs->n_fatent - 2) * fs->csize;
        fre_sect = fre_clust * fs->csize;
        total_bytes = tot_sect * fs->ssize;
        used_bytes = total_bytes - (fre_sect * fs->ssize);
    }

    char volume_name[12];
    if (f_getlabel(ESD_MOUNT_POINT, volume_name, NULL) != FR_OK) {
        httpd_resp_send_err((req), HTTPD_500_INTERNAL_SERVER_ERROR, "Error at get SD name");
        ESP_LOGE("", "Error al obtener el nombre del volumen");
        return ESP_FAIL;
    }


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
                data[buffer_counter++].size = file_info.fsize;

                buffer_size += snprintf(NULL, 0, "%s", entry->d_name);
                buffer_size += snprintf(NULL, 0, "%lu", file_info.fsize);
                
                if (buffer_counter >= MAX_FILES)
                    break;
            }
        }
    }

    closedir(dir);

    buffer_size += 4 * buffer_counter;

    buffer_size += snprintf(NULL, 0, "%s", volume_name);
    buffer_size += snprintf(NULL, 0, "%llu %llu", total_bytes, used_bytes);
    
    eStr str;
    eStr str1;
    
    eweb_add_str_urlencoded(&str,"volume_name",volume_name,false,false);
    
    ESP_LOGW("","%llu %llu",used_bytes,total_bytes);
    
    ESTR_COPY_FORMAT(&str1,"%llu",used_bytes);
    eweb_add_str_urlencoded(&str,"used_bytes",str1.data,true,false);
    
    ESP_LOGW("","%s %s",str.data,str1.data);
    ESTR_COPY_FORMAT(&str1,"%llu",total_bytes);
    eweb_add_str_urlencoded(&str,"total_bytes",str1.data,true,false);
    ESP_LOGW("","%s %s",str.data,str1.data);
    for (unsigned i = 0; i < buffer_counter; i++) {
        eweb_add_str_urlencoded(&str,"filename",data[i].filename,true,false);
        
        
        ESTR_COPY_FORMAT(&str1,"%lu",data[i].size);
        eweb_add_str_urlencoded(&str,"filesize",str1.data,true,false);
    }
    

    httpd_resp_set_type(req, "application/x-www-form-urlencoded");
    eweb_send_resp_try_chunk(req, str.data,str.length );
    estr_free(&str);
    estr_free(&str1);
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