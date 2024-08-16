idf_component_register(SRCS "espnow_transmit.c" 
INCLUDE_DIRS "." "${IDF_PATH}/components/esp_wifi/include" "${IDF_PATH}/components/nvs_flash/include"
REQUIRES "esp_wifi" "nvs_flash")

