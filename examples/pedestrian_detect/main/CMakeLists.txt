set(srcs "app_main.cpp")

set(include_dirs    ./)

set(requires        pedestrian_detect nvs_flash esp_http_client esp_wifi esp_event button esp_lcd espressif__esp32_s3_eye)

idf_component_register(SRCS ${srcs} INCLUDE_DIRS ${include_dirs} REQUIRES ${requires})
