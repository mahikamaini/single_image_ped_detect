#include "esp_log.h"
#include "pedestrian_detect.hpp"
#include "bsp/esp-bsp.h"
#include <stdio.h> 
#include <stdlib.h> 
#include "esp_heap_caps.h"
#include "dl_image_define.hpp"
#include "dl_image_jpeg.hpp"
#include "dl_image_jpeg.h"


#define IMAGE_PATH "/sdcard/IMG_0011.JPG"
const char *TAG = "pedestrian_detect";

extern "C" void jpeg_set_config(jpeg_config_t config);


dl::image::img_t resize(const dl::image::img_t &src, int target_w, int target_h) {
    const int src_w = src.width;
    const int src_h = src.height;
    const int channels = 3; // RGB888

    float scale_x = static_cast<float>(src_w) / target_w;
    float scale_y = static_cast<float>(src_h) / target_h;

    uint8_t *resized_data = (uint8_t *)heap_caps_malloc(target_w * target_h * channels, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!resized_data) {
        ESP_LOGE("resize", "Failed to allocate memory for resized image");
        dl::image::img_t empty = {nullptr, 0, 0, dl::image::DL_IMAGE_PIX_TYPE_RGB888};
        return empty;
    }

    const uint8_t *src_data = static_cast<const uint8_t *>(src.data);

    for (int y = 0; y < target_h; ++y) {
        for (int x = 0; x < target_w; ++x) {
            int src_x = static_cast<int>(x * scale_x);
            int src_y = static_cast<int>(y * scale_y);

            if (src_x >= src_w) src_x = src_w - 1;
            if (src_y >= src_h) src_y = src_h - 1;

            const uint8_t *src_pixel = src_data + (src_y * src_w + src_x) * channels;
            uint8_t *dst_pixel = resized_data + (y * target_w + x) * channels;

            dst_pixel[0] = src_pixel[0];
            dst_pixel[1] = src_pixel[1];
            dst_pixel[2] = src_pixel[2];
        }
    }

    dl::image::img_t resized_img;
    resized_img.data = resized_data;
    resized_img.width = static_cast<uint16_t>(target_w);
    resized_img.height = static_cast<uint16_t>(target_h);
    resized_img.pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB888;
    return resized_img;
}

extern "C" void app_main(void)

{

#if CONFIG_PEDESTRIAN_DETECT_MODEL_IN_SDCARD
ESP_ERROR_CHECK(bsp_sdcard_mount());
#endif

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
jpeg_set_config((jpeg_config_t){
    .out_buffer_caps = MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
});
auto img = sw_decode_jpeg(jpeg_img, dl::image::DL_IMAGE_PIX_TYPE_RGB888);
ESP_LOGE(TAG, "image is decoded");
dl::image::img_t resized_img = resize(img, 240, 240); 
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
heap_caps_free(image_buffer);
heap_caps_free(resized_img.data);

#if CONFIG_PEDESTRIAN_DETECT_MODEL_IN_SDCARD
ESP_ERROR_CHECK(bsp_sdcard_unmount());
#endif

}