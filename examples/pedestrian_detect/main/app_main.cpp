#include "esp_log.h"
#include "pedestrian_detect.hpp"
#include "bsp/esp-bsp.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h>
#include "esp_heap_caps.h"
#include "dl_image_define.hpp"
#include "dl_image_jpeg.hpp"
#include "dl_image_process.hpp"

#define IMAGE_PATH "/sdcard/IMG_0011.JPG"
const char *TAG = "pedestrian_detect";

extern "C" void app_main(void) {

// #if CONFIG_PEDESTRIAN_DETECT_MODEL_IN_SDCARD
// ESP_ERROR_CHECK(bsp_sdcard_mount());
// #endif

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
ESP_LOGE(TAG, "starting image decoding");
ESP_LOGI(TAG, "Free SPIRAM before decode: %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
auto img = sw_decode_jpeg(jpeg_img, dl::image::DL_IMAGE_PIX_TYPE_RGB565);
ESP_LOGE(TAG, "image is decoded");
dl::image::img_t resized_img;
resized_img.width = 240;
resized_img.height = 240;
resized_img.pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB888;
resized_img.data = heap_caps_malloc(240 * 240 * 3, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
if (!resized_img.data) {
        ESP_LOGE(TAG, "Failed to allocate memory for resized_img.data in SPIRAM");
        heap_caps_free(image_buffer);
        heap_caps_free(img.data);
        return;
    }
dl::image::resize(img, resized_img, dl::image::DL_IMAGE_INTERPOLATE_BILINEAR);
ESP_LOGE(TAG, "decoding done!");

PedestrianDetect *detect = new PedestrianDetect();

auto &detect_results = detect->run(resized_img);

ESP_LOGE(TAG, "model is going to run");
for (const auto &res : detect_results) { // add condition to print out "no pedestrian detected" otherwise if detect_results size = 0, or empty
ESP_LOGI(TAG, "[score: %f, x1: %d, y1: %d, x2: %d, y2: %d]", // put this in txt file
res.score,
res.box[0],
res.box[1],
res.box[2],
res.box[3]);

}
ESP_LOGE(TAG, "model is done running");

delete detect;
heap_caps_free(img.data);
ESP_LOGI(TAG, "Freed img.data");
heap_caps_free(resized_img.data);
ESP_LOGI(TAG, "Freed resized_img.data");
heap_caps_free(image_buffer);
ESP_LOGI(TAG, "Freed image_buffer");

#if CONFIG_PEDESTRIAN_DETECT_MODEL_IN_SDCARD
ESP_ERROR_CHECK(bsp_sdcard_unmount());
#endif

ESP_LOGI(TAG, "Restarting ESP32 to free memory...");
esp_restart();

}