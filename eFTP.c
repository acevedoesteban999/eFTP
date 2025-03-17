#include "eFTP.h"

#include <dirent.h>
#include "ff.h"
#include "esp_log.h"
#include "diskio.h"

#define MAX_FILES 10
#define FILE_CHUNK_SIZE 5120

int TOTAL_SD_FILES_COUNTER = 0;
int MAX_SD_FILES_INDEX = 0;
volatile bool in_proccess = false;

void eftp_init(){
    _refresh_files();
}


bool _refresh_files(){
    DIR* dir = opendir(ESD_MOUNT_POINT);
    if (dir == NULL) {
        TOTAL_SD_FILES_COUNTER = 0;
        MAX_SD_FILES_INDEX = 1;
        return false;    
    }

    struct dirent* entry;
    TOTAL_SD_FILES_COUNTER = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            TOTAL_SD_FILES_COUNTER++;
        }
    }
    MAX_SD_FILES_INDEX = (TOTAL_SD_FILES_COUNTER/MAX_FILES) + 1;
    
    closedir(dir);
    return true;
}

esp_err_t refresh_files_uri_handler(httpd_req_t *req) {
    if(!_refresh_files())
        EWEB_RETURN_ERROR_500(req,NULL,"Error at open SD");
    return ESP_OK;
}

esp_err_t eftp_uri_handler(httpd_req_t *req){
    
    eSTR str,str1;
    ESTR_MULTIPLE_INIT(
        &str,
        &str1
    );
    
    eFree efree;
    efree_init(&efree);
    EFREE_MULTIPLE_PUSH(&efree,estr_free,
        &str,
        &str1
    );

    if(esd_has_error() && esd_get_error() != 4){
        estr_copy_format(&str1,"setError('%s');",SD_STR);
        estr_copy_format(&str,ftp_min_html_asm_start,str1.ptr_char);
    }
    else{
        estr_copy_format(&str1,"getFTPData(%i);",MAX_SD_FILES_INDEX);
        estr_copy_format(&str,ftp_min_html_asm_start,str1.ptr_char);
    }
    
    httpd_resp_set_type(req, "text/html");
    eweb_send_resp_ui_str(req, &str);
    efree_free(&efree);
    return ESP_OK;
}

esp_err_t eftp_get_data_post_handler(httpd_req_t *req) {
    FATFS *fs;
    DWORD fre_clust, fre_sect, tot_sect;
    uint64_t total_bytes = 0,used_bytes = 0;
    
    if(esd_has_error()){
        if(esd_get_error()  == -1){
            if(esd_init() != ESP_OK)
                return ESP_FAIL;
            else
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
        EWEB_RETURN_ERROR_500(req,NULL,"Error at get SD name");
    }

    DIR* dir = opendir(ESD_MOUNT_POINT);
    
    if (dir == NULL) {
        EWEB_RETURN_ERROR_500(req,NULL,"Error at open SD directory");
    }

    eftp_data* ftp_data = malloc(  MAX_FILES * sizeof(eftp_data));
    eftp_data* ftp_data_ordened = malloc( MAX_FILES * sizeof(eftp_data));
    if (ftp_data == NULL || ftp_data_ordened == NULL) {
        closedir(dir);
        EWEB_RETURN_ERROR_500(req,NULL,"Error at open alocate ftpdata");
    }

    int ftp_data_counter = 0 ,ftp_data_counter_ordened = 0;
    int file_counter = 0;
    struct dirent* entry;
    FILINFO file_info;

    eSTR str,str1,str2;
    ESTR_MULTIPLE_INIT(
        &str,
        &str1,
        &str2
    );

    eFree efree;
    efree_init(&efree);
    EFREE_MULTIPLE_PUSH(&efree,estr_free,
        &str,
        &str1,
        &str2
    );

    EWEB_GET_DATA_REQUEST_STR(req, &str2, &efree);
    
    int index;
    eweb_get_int_urlencoded(str2.ptr_char, "index", &index);
    if(index < 0 || index > MAX_SD_FILES_INDEX){
        EWEB_RETURN_ERROR_500(req,&efree,"Index no valid");
    }
    

    int offset;
    if (index == 0) 
        offset = MAX_SD_FILES_INDEX - MAX_FILES  - 1;
    else
        offset = (index - 1) * MAX_FILES - 1;

    file_counter = 0;
    ftp_data_counter = 0;
    while ((entry = readdir(dir)) != NULL && ftp_data_counter < MAX_FILES) {
        if (entry->d_type == DT_REG) {
            if (file_counter > offset) {
                FRESULT f = f_stat(entry->d_name, &file_info);
                if (f == FR_OK) {
                    strncpy(ftp_data[ftp_data_counter].filename, entry->d_name,sizeof(ftp_data[ftp_data_counter].filename));
                    ftp_data[ftp_data_counter].size = file_info.fsize;
                    if(ftp_data_counter++ >= MAX_FILES)
                        break;
                }
            }
            file_counter++;
        }
    }

    for (int i = ftp_data_counter - 1; i >= 0; i--) {
        memcpy(&ftp_data_ordened[ftp_data_counter - 1 - i], 
               &ftp_data[i], sizeof(eftp_data));
    }

    closedir(dir);
    eweb_add_str_urlencoded(&str,"volume_name",volume_name,false,false);
    
    estr_copy_format(&str1,"%llu",used_bytes);
    eweb_add_str_urlencoded(&str,"used_bytes",str1.ptr_char,true,false);
    
    estr_copy_format(&str1,"%llu",total_bytes);
    eweb_add_str_urlencoded(&str,"total_bytes",str1.ptr_char,true,false);

    for (unsigned i = 0; i < ftp_data_counter_ordened; i++) {
        eweb_add_str_urlencoded(&str,"filename",ftp_data_ordened[i].filename,true,false);
        
        estr_copy_format(&str1,"%lu",ftp_data_ordened[i].size);
        eweb_add_str_urlencoded(&str,"filesize",str1.ptr_char,true,false);
    }
    
    estr_copy_format(&str2,"%i",MAX_SD_FILES_INDEX);
    eweb_add_str_urlencoded(&str,"max_index",str2.ptr_char,true,false);
    

    httpd_resp_set_type(req, "application/x-www-form-urlencoded");
    eweb_send_resp_buff(req, str.ptr_char,str.length );
    efree_free(&efree);

    return ESP_OK;
}


esp_err_t eftp_get_file_post_handler(httpd_req_t *req){
    eSTR str,str1,filename_str,path_str;
    ESTR_MULTIPLE_INIT(
        &str,
        &str1,
        &filename_str,
        &path_str
    );

    eFree efree;
    efree_init(&efree);
    EFREE_MULTIPLE_PUSH(&efree,estr_free,
        &str,
        &str1,
        &filename_str,
        &path_str
    );
    EWEB_GET_DATA_REQUEST_STR(req, &str,&efree);
    EWEB_CHECK_STR_URLENCODED(req,str.ptr_char,"filename",&filename_str,&efree);
    
    estr_append_str(&path_str,false,ESD_MOUNT_POINT);
    estr_append_str(&path_str,false,"/");
    estr_append_str(&path_str,false,filename_str.ptr_char);

    FILE *file = NULL;
    error_t err = ESP_OK;
    file = fopen(path_str.ptr_char, "rb");
    if (!file) {
        ESP_LOGE("","File not found: %s", path_str.ptr_char);
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File not found");
        efree_free(&efree);
        return ESP_FAIL;
    }

    if (!estr_prepare_str(&str1,FILE_CHUNK_SIZE)) {
        ESP_LOGE("", "Failed to allocate memory for file chunk");
        fclose(file);
        efree_free(&efree);
        return ESP_FAIL;
    }
    httpd_resp_set_type(req, "application/octet-stream");
    
    size_t read_bytes;
    while ((read_bytes = fread(str1.ptr_char, 1, FILE_CHUNK_SIZE, file)) > 0) {
        if (httpd_resp_send_chunk(req, str1.ptr_char, read_bytes) != ESP_OK) {
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
    
    fclose(file);
    efree_free(&efree);
    return err;
}



esp_err_t eftp_delete_file_post_handler(httpd_req_t *req){
    eSTR str,str1,path_str;
    ESTR_MULTIPLE_INIT(
        &str,
        &str1,
        &path_str
    );

    eFree efree;
    efree_init(&efree);
    EFREE_MULTIPLE_PUSH(
        &efree,
        estr_free,
        &str,
        &str1,
        &path_str
    );

    EWEB_GET_DATA_REQUEST_STR(req,&str,&efree);
    EWEB_CHECK_STR_URLENCODED(req,str.ptr_char,"filename",&str1,&efree);

    estr_append_str(&path_str,false,ESD_MOUNT_POINT);
    estr_append_str(&path_str,false,"/");
    estr_append_str(&path_str,false,str1.ptr_char);

    if(esd_delete_file(path_str.ptr_char)){
        httpd_resp_set_type(req, "text/html");
        httpd_resp_send(req, "File Deleted", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }
    else{

        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal Server Error");
        return ESP_FAIL;
    }


}