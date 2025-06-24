#include "esp_log.h"
#include "pedestrian_detect.hpp"
#include "bsp/esp-bsp.h"
#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include "esp_heap_caps.h"
#include "dl_image_define.hpp"
#include "dl_image_jpeg.hpp"
#include "dl_image_process.hpp"

// #define image_path "/sdcard/IMG_0015.JPG"
const char *TAG = "pedestrian_detect";

extern "C" void app_main(void) {

ESP_ERROR_CHECK(bsp_sdcard_mount());
ESP_LOGI(TAG, "SD card is mounted!");

DIR *dir = opendir("/sdcard");
if (!dir) {
    ESP_LOGE(TAG, "Failed to open /sdcard: %s", strerror(errno));
    return;
}

struct dirent *entry;
PedestrianDetect *detect = new PedestrianDetect();
// int image_count = 0;
while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type != DT_REG) continue; // Skip if not a regular file

    const char *fname = entry->d_name;
    
    if (strncmp(fname, "._", 2) == 0 || fname[0] == '.') continue;
    if (!(strstr(fname, ".JPG") || strstr(fname, ".jpg"))) continue; // Only JPGs
    
    char image_path[272];
    snprintf(image_path, sizeof(image_path), "/sdcard/%s", fname);

    FILE *file = fopen(image_path, "rb");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open image file: %s", strerror(errno));
        continue;
    }
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);
    uint8_t *image_buffer = (uint8_t *)heap_caps_malloc(file_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    if (!image_buffer) {
        ESP_LOGE(TAG, "Failed to allocate memory for image");
        fclose(file);
        continue;
    }
    fread(image_buffer, 1, file_size, file);
    fclose(file);
    dl::image::jpeg_img_t jpeg_img = {
        .data = image_buffer,
        .data_len = file_size
    };

    // decode jpeg into image we can use
    ESP_LOGI(TAG, "starting image decoding");
    ESP_LOGI(TAG, "Free SPIRAM before decode: %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    auto img = sw_decode_jpeg(jpeg_img, dl::image::DL_IMAGE_PIX_TYPE_RGB888, 0, 2); // currently setting scale ratio to 2 (1/2 original image size)
    ESP_LOGI(TAG, "image is decoded");
    ESP_LOGI(TAG, "Free SPIRAM after decode: %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGI(TAG, "decoding done!");

    ESP_LOGI(TAG, "going to run ped detect model");
    ESP_LOGI(TAG, "Free PSRAM before model: %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    int crop_size = img.height;  // 768
    // int crop_size = 240; // for cropping directly to 240x240
    std::vector<std::vector<int>> crop_areas = {
        { (img.width - crop_size) / 2, 0, crop_size, crop_size },   // Center crop
        { 0, 0, crop_size, crop_size },                             // Left crop
        { img.width - crop_size, 0, crop_size, crop_size }          // Right crop
    };

    float best_score = -1.0;
    std::vector<dl::detect::result_t> best_results;

    for (size_t i = 0; i < crop_areas.size(); ++i) {
        const auto& area = crop_areas[i];
        ESP_LOGI(TAG, "Trying crop #%d at x=%d", (int)i + 1, area[0]);

        dl::image::img_t cropped_img;
        cropped_img.width = 240;
        cropped_img.height = 240;
        cropped_img.pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB888;
        cropped_img.data = heap_caps_malloc(240 * 240 * 3, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (!cropped_img.data) {
            ESP_LOGE(TAG, "Failed to allocate memory for cropped_img.data in SPIRAM");
            heap_caps_free(image_buffer);
            heap_caps_free(img.data);
            delete detect;
            continue;
        }

        dl::image::resize(img, cropped_img, dl::image::DL_IMAGE_INTERPOLATE_BILINEAR, 0, nullptr, area);
        
        // Manually copy the crop region into cropped_img
        // const int channels = 3;
        // for (int y = 0; y < 240; ++y) {
        //     memcpy((uint8_t*)cropped_img.data + y * 240 * channels,
        //     (uint8_t*)img.data + ((area[1] + y) * img.width + area[0]) * channels,
        //     240 * channels);
        // }

        // Run model
        std::list<dl::detect::result_t> results = detect->run(cropped_img);

        // Pick highest scoring result
        if (!results.empty() && results.front().score > best_score) {
            best_score = results.front().score;
            best_results = std::vector<dl::detect::result_t>(results.begin(), results.end());
        }

        heap_caps_free(cropped_img.data);

        // If confidence is high, no need to try other crops
        if (best_score >= 0.85) break;
    }

    ESP_LOGI(TAG, "saving results");

    ESP_LOGI(TAG, "model is going to run and write to the .txt file");
    FILE *output_file = fopen("/sdcard/detection_results.txt", "a"); // a = append
    if (!output_file) {
        ESP_LOGE(TAG, "Failed to open output file: %s", strerror(errno));
    } else {
        if (best_results.empty() || best_results.size() == 0) {
            fprintf(output_file, "Image: %s -> No pedestrian detected\n", image_path);
        } else {
            fprintf(output_file, "Image: %s -> %zu pedestrian(s) detected:\n", image_path, best_results.size());
            for (const auto &res : best_results) {
                fprintf(output_file, "[score: %.2f, x1: %d, y1: %d, x2: %d, y2: %d]\n",
                        res.score,
                        res.box[0],
                        res.box[1],
                        res.box[2],
                        res.box[3]);
            }
        }
        fprintf(output_file, "----\n"); // Separator between images
        fclose(output_file);
    }
    ESP_LOGI(TAG, "model is done running");

    heap_caps_free(img.data);       
    heap_caps_free(image_buffer);

    // image_count++;
    // if (image_count >= 3) {
    //     ESP_LOGI(TAG, "Processed %d images, restarting to free memory...", image_count);
    //     esp_restart();
    // }    
}

delete detect;  
closedir(dir);

#if CONFIG_PEDESTRIAN_DETECT_MODEL_IN_SDCARD
ESP_ERROR_CHECK(bsp_sdcard_unmount());
#endif

}