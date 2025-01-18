idf_component_register(
    SRCS 
        "eSdWeb.c"

    INCLUDE_DIRS 
        "include"
        
    EMBED_FILES
        "src/sdweb.min.html"
        "src/js/sdweb.min.js"
    
    REQUIRES
        eSD
        eWeb
        
)
