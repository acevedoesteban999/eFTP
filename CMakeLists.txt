idf_component_register(
    SRCS 
        "eFTP.c"

    INCLUDE_DIRS 
        "include"
        
    EMBED_FILES
        "src/ftp.min.html"
        "src/js/ftp.min.js"
    
    REQUIRES
        eSD
        eWeb
        
)
