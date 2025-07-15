// initialization
#include "pedestrian_detect.hpp"
#include "who_detect_app.hpp"

using namespace who::cam;
using namespace who::lcd;
using namespace who::app;

#define WITH_LCD 1

// execution begins
extern "C" void app_main(void)
{

// if ped model is on sd card, mount it at the start.
// checks if esp_err_t value (runtime error) = ESP_OK (which is 0, meaning no error)
#if CONFIG_PEDESTRIAN_DETECT_MODEL_IN_SDCARD
    ESP_ERROR_CHECK(bsp_sdcard_mount());
#endif

// for ESP32-S3 models, setup LEDs and turn off the green LED on the physical board.
#if CONFIG_IDF_TARGET_ESP32S3
    ESP_ERROR_CHECK(bsp_leds_init());
    ESP_ERROR_CHECK(bsp_led_set(BSP_LED_GREEN, false));
#endif

// for ESP32-P4 or ESP32-S3 models, setup the camera (more details in who_cam folder under who_peripherals)
#if CONFIG_IDF_TARGET_ESP32P4
    auto cam = new WhoP4Cam(VIDEO_PIX_FMT_RGB565, 3, V4L2_MEMORY_USERPTR, true);
    // auto cam = new WhoP4PPACam(VIDEO_PIX_FMT_RGB565, 4, V4L2_MEMORY_USERPTR, 224, 224, true);
#elif CONFIG_IDF_TARGET_ESP32S3
    auto cam = new WhoS3Cam(PIXFORMAT_RGB565, FRAMESIZE_240X240, 2, true);
// create LCD object when done
#endif
    auto lcd = new WhoLCD();

#if WITH_LCD // if LCD = 1
    auto model = new PedestrianDetect(); // new instance of neural network model
    // loop: grab frames from camera, runs model on each frame, draws a red box around each person detected
    auto detect = new WhoDetectAppLCD({{255, 0, 0}});
    detect->set_cam(cam);
    detect->set_lcd(lcd);
    detect->set_model(model);
    // detect->set_fps(5); // good for saving power, debugging
    detect->run();
#else // if LCD = 0
    auto model = new PedestrianDetect();
    auto detect = new WhoDetectAppTerm(); // print pedestrian coordinates to the terminal
    detect->set_cam(cam);
    detect->set_model(model);
    // detect->set_fps(5);
    detect->run();
#endif
}