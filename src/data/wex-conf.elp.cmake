* GLOBAL:
   FORMAT               =  "%datetime{%d/%M/%Y %H:%m:%s} %level %msg"
   FILENAME             = "${CONFIG_INSTALL_DIR}/${PROJECT_NAME}.log"
   ENABLED              =  true
   TO_FILE              =  true
   TO_STANDARD_OUTPUT   =  true
   PERFORMANCE_TRACKING =  true
   MAX_LOG_FILE_SIZE    =  2097152 ## 2MB
   LOG_FLUSH_THRESHOLD  =  1
