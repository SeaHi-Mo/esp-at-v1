
set(require_components ${IDF_TARGET} mqtt mdns esp_http_client json freertos spiffs
    bootloader_support app_update openssl wpa_supplicant spi_flash esp_http_server esp_wifi nvs_flash)
file(GLOB_RECURSE srcs src/*.c)

#if (CONFIG_AT_WEB_SERVER_SUPPORT)
 #   if(NOT CONFIG_AT_WEB_USE_FATFS)
  #      set(embed_txt_files ../fs_image/index.html)
   # endif()
#endif()

idf_component_register (
    SRCS ${srcs}
    INCLUDE_DIRS include ../at/include
     REQUIRES ${require_components} 
)

