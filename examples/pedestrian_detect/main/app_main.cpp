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
#include "esp_camera.h"
#include "bsp/esp32_s3_eye.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "iot_button.h"
#include "esp_adc/adc_oneshot.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "dl_model_context.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"

static volatile bool is_menu_button_pressed = false;
static volatile bool is_play_button_pressed = false;
static int64_t menu_button_start = 0;
const char *TAG = "pedestrian_detect";

static void menu_button_cb(void *arg, void *usr_data)
{
    menu_button_start = esp_timer_get_time();
    is_menu_button_pressed = true;
}

static void play_button_cb(void *arg, void *usr_data)
{
    is_play_button_pressed = true;
}

void click_and_save_pic() {
    ESP_LOGI(TAG, "Menu button pressed! Taking a picture...");
    camera_fb_t *pic = esp_camera_fb_get();
    int64_t pic_taken_time = esp_timer_get_time();
    if (!pic) {
        ESP_LOGE(TAG, "Couldn't capture picture");
        return;
    }
    float duration_ms = (float)(pic_taken_time - menu_button_start) / 1000.0;
    ESP_LOGI(TAG, "Time from button press to image capture: %.2f ms", duration_ms);
        static int pic_count = 0;
        pic_count++;
        char filename[40];
        sprintf(filename, "/sdcard/capture_%d.jpg", pic_count);

        FILE *file = fopen(filename, "wb");
        if (file) {
            fwrite(pic->buf, 1, pic->len, file);
            ESP_LOGI(TAG, "Picture saved: %s", filename);
            fclose(file);
        } else {
            ESP_LOGE(TAG, "Could not open file for saving");
        }

    esp_camera_fb_return(pic);
}


extern "C" void app_main(void) {
    bsp_i2c_init();
    bsp_display_start();
    bsp_display_backlight_on();

    // Button setup
    button_handle_t btns[BSP_BUTTON_NUM] = {NULL};
    ESP_ERROR_CHECK(bsp_iot_button_create(btns, NULL, BSP_BUTTON_NUM));

    // Initialize the camera
    int64_t start_time = esp_timer_get_time();
    camera_config_t camera_config = BSP_CAMERA_DEFAULT_CONFIG;
    camera_config.pixel_format = PIXFORMAT_JPEG;      // <-- Key change: Use JPEG from the start
    camera_config.frame_size = FRAMESIZE_240X240;     // Match LCD resolution
    camera_config.jpeg_quality = 12;           // Quality for preview and capture
    camera_config.fb_count = 2;                      // Use 2 frame buffers for smoother capture
    camera_config.fb_location = CAMERA_FB_IN_PSRAM;
    camera_config.grab_mode = CAMERA_GRAB_LATEST;
    camera_config.xclk_freq_hz = 16500000;
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
        return;
    }

    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, BSP_CAMERA_VFLIP);
    s->set_hmirror(s, BSP_CAMERA_HMIRROR);
    int64_t end_time = esp_timer_get_time();
    ESP_LOGI(TAG, "Camera Init done");
    ESP_LOGI(TAG, "Camera initialized in %lld microseconds", end_time - start_time);

    uint32_t cam_buff_size = BSP_LCD_H_RES * BSP_LCD_V_RES * 2;
    uint8_t *cam_buff = (uint8_t *) heap_caps_malloc(cam_buff_size, MALLOC_CAP_SPIRAM);
    assert(cam_buff);

    ESP_ERROR_CHECK(bsp_sdcard_mount());

    // Create LVGL canvas for camera image
    bsp_display_lock(0);
    lv_obj_t *camera_canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(camera_canvas, cam_buff, BSP_LCD_H_RES, BSP_LCD_V_RES, LV_COLOR_FORMAT_RGB565);
    assert(camera_canvas);
    lv_obj_center(camera_canvas);
    bsp_display_unlock();

    iot_button_register_cb(btns[BSP_BUTTON_MENU], BUTTON_SINGLE_CLICK, NULL, menu_button_cb, NULL);
    iot_button_register_cb(btns[BSP_BUTTON_PLAY], BUTTON_SINGLE_CLICK, NULL, play_button_cb, NULL);

    ESP_LOGI(TAG, "Starting live preview...");
    while (true) {
        if (is_play_button_pressed) {
            break;
        }
         if (is_menu_button_pressed) {
            is_menu_button_pressed = false;
            click_and_save_pic();
            continue;
        }

        camera_fb_t *pic = esp_camera_fb_get();
        if (pic) {
            bool converted = jpg2rgb565(pic->buf, pic->len, cam_buff, JPG_SCALE_NONE);
            if (converted) {
                bsp_display_lock(0);
                lv_obj_invalidate(camera_canvas);
                bsp_display_unlock();
            } else {
                ESP_LOGE(TAG, "JPEG conversion failed!");
            }
            // Return the buffer only when it's valid and you're done with it.
            esp_camera_fb_return(pic); 
        } else {
            ESP_LOGE(TAG, "Get frame failed");
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    ESP_LOGI(TAG, "Exiting preview loop.");

DIR *dir = opendir("/sdcard");
if (!dir) {
    ESP_LOGE(TAG, "Failed to open /sdcard: %s", strerror(errno));
    return;
}

struct dirent *entry;
PedestrianDetect *detect = new PedestrianDetect();
dl::image::img_t cropped_img;
cropped_img.width = 240;
cropped_img.height = 240;
cropped_img.pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB888;
cropped_img.data = heap_caps_malloc(240 * 240 * 3, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
if (!cropped_img.data) {
    ESP_LOGE(TAG, "Failed to allocate memory for cropped_img.data in SPIRAM");
    return;
}

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
    auto img = sw_decode_jpeg(jpeg_img, dl::image::DL_IMAGE_PIX_TYPE_RGB888, 0);
    // ESP_LOGI(TAG, "Image %d of %zu processing.", i + 1, image_paths.size());
    if (!img.data) {
        ESP_LOGE(TAG, "Failed to decode image, skipping.");
        heap_caps_free(image_buffer); // Free the original JPEG buffer
        continue;                     // Skip to the next image
    }
    ESP_LOGI(TAG, "Free SPIRAM after decode: %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    ESP_LOGI(TAG, "Free PSRAM before model: %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    int crop_size = img.height;  
    std::vector<std::vector<int>> crop_areas = {
        { (img.width - crop_size) / 2, 0, crop_size, crop_size },   // Center crop
        { 0, 0, crop_size, crop_size },                             // Left crop
        { img.width - crop_size, 0, crop_size, crop_size }          // Right crop
    };

    float best_score = -1.0;
    std::vector<dl::detect::result_t> best_results;

    int crop_number = 0;
    for (size_t i = 0; i < crop_areas.size(); ++i) {
        const auto& area = crop_areas[i];
        ESP_LOGI(TAG, "Trying crop #%d at x=%d", (int)i + 1, area[0]);

        dl::image::resize(img, cropped_img, dl::image::DL_IMAGE_INTERPOLATE_BILINEAR, 0, nullptr, area);

        // Run model
        std::list<dl::detect::result_t> results = detect->run(cropped_img);

        // Pick highest scoring result
        if (!results.empty() && results.front().score > best_score) {
            best_score = results.front().score;
            best_results = std::vector<dl::detect::result_t>(results.begin(), results.end());
            crop_number = (int) i;
        }

        // If confidence is high, no need to try other crops
        if (best_score >= 0.85) break;
    }
    
    ESP_LOGI(TAG, "saving results");
    FILE *output_file = fopen("/sdcard/detection_results.txt", "a"); // a = append
    if (!output_file) {
        ESP_LOGE(TAG, "Failed to open output file: %s", strerror(errno));
        return;
    }   
        if (best_results.empty() || best_results.size() == 0) {
            fprintf(output_file, "Image: %s -> No pedestrian detected\n", image_path);
        } else {
            fprintf(output_file, "Image: %s -> %zu pedestrian(s) detected:\n", image_path, best_results.size());
            for (const auto &res : best_results) {
                fprintf(output_file, "[score: %.2f, x1: %d, y1: %d, x2: %d, y2: %d]\n",
                        res.score,
                        res.box[0] + crop_areas[crop_number][0],
                        res.box[1]  + crop_areas[crop_number][1],
                        res.box[2] + crop_areas[crop_number][0],
                        res.box[3] + crop_areas[crop_number][1]);
            }
        }
        fprintf(output_file, "----\n"); // Separator between images
        fclose(output_file);
        heap_caps_free(img.data);       
        heap_caps_free(image_buffer);

} 

heap_caps_free(cropped_img.data);
closedir(dir);
delete detect;  
ESP_ERROR_CHECK(bsp_sdcard_unmount());
ESP_LOGI(TAG, "Processing complete. SD card unmounted.");

#if CONFIG_PEDESTRIAN_DETECT_MODEL_IN_SDCARD
ESP_ERROR_CHECK(bsp_sdcard_unmount());
#endif

}