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
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "dl_model_context.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <inttypes.h>

#define WIFI_SSID "maini-IoT"
#define WIFI_PWD "18112000"
#define SERVER_IP "192.168.15.5"
#define SERVER_PORT "8000"
#define EXAMPLE_ESP_MAXIMUM_RETRY 5
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD "WPA2 PSK"
#define ESP_WIFI_SAE_MODE "BOTH"
#define EXAMPLE_H2E_IDENTIFIER "H2E"

const char *TAG = "pedestrian_detect";

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;

#define MIN_FREE_SPIRAM 2621440 

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PWD
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 WIFI_SSID, WIFI_PWD);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 WIFI_SSID, WIFI_PWD);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}


extern "C" void app_main(void) {

  //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();


// NVS handle
nvs_handle_t nvs_handle;
esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
// nvs_erase_key(nvs_handle, "img_idx"); // run to start over from beginning
if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
}

// Read the last processed image index
int32_t start_index = 0; // Default to 0
err = nvs_get_i32(nvs_handle, "img_idx", &start_index);
switch (err) {
    case ESP_OK:
        ESP_LOGI(TAG, "Resuming from image index %" PRId32, start_index);
        break;
    case ESP_ERR_NVS_NOT_FOUND:
        ESP_LOGI(TAG, "First run, starting from index 0");
        break;
    default :
        ESP_LOGE(TAG, "Error reading NVS: %s", esp_err_to_name(err));
}

// The image list is downloaded here...

uint8_t mac_addr[6] = {0};
    esp_wifi_get_mac(WIFI_IF_STA, mac_addr);
    ESP_LOGI(TAG, "ESP32 MAC Address: %02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2],
             mac_addr[3], mac_addr[4], mac_addr[5]);

std::vector<std::string> image_paths;
std::string file_list_buffer;

char list_url[100];
snprintf(list_url, sizeof(list_url), "http://%s:%s/image_list.txt", SERVER_IP, SERVER_PORT);

esp_http_client_config_t config_list = { .url = list_url };
esp_http_client_handle_t client_list = esp_http_client_init(&config_list);
esp_http_client_open(client_list, 0);
int content_length = esp_http_client_fetch_headers(client_list);
file_list_buffer.resize(content_length);
esp_http_client_read(client_list, &file_list_buffer[0], content_length);
esp_http_client_close(client_list);
esp_http_client_cleanup(client_list);

std::stringstream ss(file_list_buffer);
std::string line;

while (std::getline(ss, line)) {
    if (line.length() > 1) {
       if (line.back() == '\r') {
        line.pop_back();
       }
       image_paths.push_back(line); 
    }
}

ESP_LOGI(TAG, "Successfully downloaded and parsed image list. Found %zu images.", image_paths.size());

ESP_ERROR_CHECK(bsp_sdcard_mount());
ESP_LOGI(TAG, "SD card is mounted!");

// DIR *dir = opendir("/sdcard");
// if (!dir) {
//     ESP_LOGE(TAG, "Failed to open /sdcard: %s", strerror(errno));
//     return;
// }

// struct dirent *entry;
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

int img_count = 0;
// 3. Loop through each image path, download, and process
    for (int i = start_index; i < image_paths.size(); i++) {
        const auto& image_path = image_paths[i];
        if (heap_caps_get_free_size(MALLOC_CAP_SPIRAM) < MIN_FREE_SPIRAM) {
        ESP_LOGE(TAG, "Memory low (%d bytes). Restarting to prevent crash.", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        esp_restart();
    }
        char image_url[272];
        // The find command on Mac/Linux prefixes with './', let's handle that
        const char* path_to_use = image_path.c_str();
        if (strncmp(path_to_use, "./", 2) == 0) {
            path_to_use += 2;
        }
        snprintf(image_url, sizeof(image_url), "http://%s:%s/%s", SERVER_IP, SERVER_PORT, path_to_use);
        ESP_LOGI(TAG, "Processing image: %s", image_url);

        ESP_LOGI(TAG, "Free SPIRAM before decode: %d bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        
        uint8_t *image_buffer = nullptr;
        
        esp_http_client_config_t config_img = { .url = image_url, .timeout_ms = 15000 };
        esp_http_client_handle_t client_img = esp_http_client_init(&config_img);

        if (esp_http_client_open(client_img, 0) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to open HTTP connection to %s", image_url);
            esp_http_client_cleanup(client_img);
            continue;
        }

        int img_len = esp_http_client_fetch_headers(client_img);
        if (img_len <= 0) {
            ESP_LOGE(TAG, "Failed to get content length for %s", image_url);
            esp_http_client_close(client_img);
            esp_http_client_cleanup(client_img);
            continue;
        }
        
        image_buffer = (uint8_t *)heap_caps_malloc(img_len, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (!image_buffer) {
            ESP_LOGE(TAG, "Failed to allocate memory for image");
            esp_http_client_close(client_img);
            esp_http_client_cleanup(client_img);
            continue;
        }

        esp_http_client_read_response(client_img, (char*)image_buffer, img_len);
        esp_http_client_close(client_img);
        esp_http_client_cleanup(client_img);

// while ((entry = readdir(dir)) != NULL) {
//     if (entry->d_type != DT_REG) continue; // Skip if not a regular file

//     const char *fname = entry->d_name;
    
//     if (strncmp(fname, "._", 2) == 0 || fname[0] == '.') continue;
//     if (!(strstr(fname, ".JPG") || strstr(fname, ".jpg"))) continue; // Only JPGs
    
//     char image_path[272];
//     snprintf(image_path, sizeof(image_path), "/sdcard/%s", fname);

//     FILE *file = fopen(image_path, "rb");
//     if (!file) {
//         ESP_LOGE(TAG, "Failed to open image file: %s", strerror(errno));
//         continue;
//     }
//     fseek(file, 0, SEEK_END);
//     size_t file_size = ftell(file);
//     rewind(file);
//     uint8_t *image_buffer = (uint8_t *)heap_caps_malloc(file_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

//     if (!image_buffer) {
//         ESP_LOGE(TAG, "Failed to allocate memory for image");
//         fclose(file);
//         continue;
//     }
//     fread(image_buffer, 1, file_size, file);
//     fclose(file);

    dl::image::jpeg_img_t jpeg_img = {
        .data = image_buffer,
        .data_len = (size_t) img_len
    };

    // decode jpeg into image we can use
    auto img = sw_decode_jpeg(jpeg_img, dl::image::DL_IMAGE_PIX_TYPE_RGB888, 0);
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

        // detect->cleanup();

        // If confidence is high, no need to try other crops
        if (best_score >= 0.85) break;
    }
    // what if image is too large?
    ESP_LOGI(TAG, "saving results");
    FILE *output_file = fopen("/sdcard/detection_results.txt", "a"); // a = append
    if (!output_file) {
        ESP_LOGE(TAG, "Failed to open output file: %s", strerror(errno));
        return;
    }   
        if (best_results.empty() || best_results.size() == 0) {
            fprintf(output_file, "Image: %s -> No pedestrian detected\n", image_path.c_str());
        } else {
            fprintf(output_file, "Image: %s -> %zu pedestrian(s) detected:\n", image_path.c_str(), best_results.size());
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
        ESP_LOGI(TAG, "Image %d of %zu processed. Saving progress.", i + 1, image_paths.size());
    err = nvs_set_i32(nvs_handle, "img_idx", i + 1); // Save the NEXT index
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to commit NVS changes!");
    }
        img_count++;
        // ESP_LOGE(TAG, "Image %d of %zu processed", img_count, image_paths.size());

        heap_caps_free(img.data);       
        heap_caps_free(image_buffer);
    }  

heap_caps_free(cropped_img.data);
nvs_close(nvs_handle);
delete detect;  
ESP_ERROR_CHECK(bsp_sdcard_unmount());
ESP_LOGI(TAG, "Processing complete. SD card unmounted.");
// closedir(dir);

#if CONFIG_PEDESTRIAN_DETECT_MODEL_IN_SDCARD
ESP_ERROR_CHECK(bsp_sdcard_unmount());
#endif

}