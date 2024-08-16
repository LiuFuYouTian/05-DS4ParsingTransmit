idf_component_register(SRCS "ps4_control.c" 
                            "ps4_l2cap.c"
                            "ps4_spp.c"
                            INCLUDE_DIRS "." "${IDF_PATH}/components/bt/include/esp32/include"
                            REQUIRES "bt")

