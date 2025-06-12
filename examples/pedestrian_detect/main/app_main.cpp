#include "esp_log.h"
#include "pedestrian_detect.hpp"
#include "bsp/esp-bsp.h"
#include <stdio.h> 
#include <stdlib.h> 

// where image is embedded in memory (not needed for sd card loading)
// extern const uint8_t pedestrian_jpg_start[] asm("_binary_pedestrian_jpg_start");
// extern const uint8_t pedestrian_jpg_end[] asm("_binary_pedestrian_jpg_end");

#define IMAGE_PATH "/sdcard/IMG_0106.JPG"
const char *TAG = "pedestrian_detect";




extern "C" void app_main(void)

{

#if CONFIG_PEDESTRIAN_DETECT_MODEL_IN_SDCARD
ESP_ERROR_CHECK(bsp_sdcard_mount());
#endif

// gets pic data from memory - change this to get pic data from sd card
// dl::image::jpeg_img_t jpeg_img = {.data = (void *)pedestrian_jpg_start,
// .data_len = (size_t)(pedestrian_jpg_end - pedestrian_jpg_start)};

ESP_ERROR_CHECK(bsp_sdcard_mount());
ESP_LOGE(TAG, "SD card is mounted!");

FILE *file = fopen(IMAGE_PATH, "rb");
if (!file) {
    ESP_LOGE(TAG, "Failed to open image file: %s", strerror(errno));
    return;
}
fseek(file, 0, SEEK_END);
size_t file_size = ftell(file);
rewind(file);
uint8_t *image_buffer = (uint8_t *)heap_caps_malloc(file_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
if (!image_buffer) {
    ESP_LOGE(TAG, "Failed to allocate memory for image");
    fclose(file);
    return;
}
fread(image_buffer, 1, file_size, file);
fclose(file);
dl::image::jpeg_img_t jpeg_img = {
    .data = image_buffer,
    .data_len = file_size
};

// decode jpeg into image we can use
ESP_LOGE(TAG, "trying to decode image");
// dl::image::img_t resized_img = resize(jpeg_img, 240, 280); // need to write
auto img = sw_decode_jpeg(jpeg_img, dl::image::DL_IMAGE_PIX_TYPE_RGB888);

PedestrianDetect *detect = new PedestrianDetect();

auto &detect_results = detect->run(img);

for (const auto &res : detect_results) {
ESP_LOGI(TAG, "[score: %f, x1: %d, y1: %d, x2: %d, y2: %d]",
res.score,
res.box[0],
res.box[1],
res.box[2],
res.box[3]);

}

delete detect;
heap_caps_free(image_buffer);

#if CONFIG_PEDESTRIAN_DETECT_MODEL_IN_SDCARD
ESP_ERROR_CHECK(bsp_sdcard_unmount());
#endif

}