idf_component_register(
    SRCS 
        "eSdWeb.c"

    INCLUDE_DIRS 
        "include"
        
    EMBED_FILES
        "src/sd.html"
    
    REQUIRES
        eSD
        eWeb
        
)
