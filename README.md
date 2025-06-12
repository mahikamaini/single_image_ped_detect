# TinkerTrap Notebook
## Tuesday, May 13, 2025
### Task
Set up ESP-IDF code environment in VSCode on my Mac. 
### Notes
In order to test if the code runs, the ESP-IDF will need to be plugged into my laptop. This should arrive by the end of the week. 
### What I Accomplished
I followed a provided video tutorial to integrate the necessary software with the ESP-IDF VSCode extension. While it was meant for Windows, I was able to successfully integrate it with MacOS. I also cloned the GitHub repository for the ESP-IDF code, including the pedestrian-detect sample we will be using, and created a new repository, TinkerTrap, for our purposes. 

## Wednesday, May 14, 2025
### Task
Read through and understand the pedestrian_detect example code from the original GitHub repository.
### Notes
I commented all observations in the app_main.cpp file under the pedestrian_detect folder.
### What I Accomplished
I read through the example code and broke it down to understand its functionality. While I don't have a background in C++, I utilized the Espressif API and ChatGPT to break down some of the syntax in order to better understand the logical function. As a result, I'm a little more familiar with the code function, though not necessarily how to adapt it to our specific purpose quite yet. 

## Monday, May 19, 2025
### Task
Plug in and set up the ESP32-S3 device. 
### Notes
The device requires a USB-A to micro-USB cable, not a USB-A to USB-C cable. 
### What I Accomplished
I was able to install the necessary UART drivers on my laptop and obtain a USB-A to micro-USB cable. From there, I connected the ESP32 to my laptop and successfully flashed some of the example scripts to it. With Ben, we also discussed how to best adapt the existing pedestrian_detect code to apply to single images rather than a continuous video feed using ChatGPT as a guide. 

## Wednesday, May 21, 2025
### Task
Do initial file structure setup for the project as given by ChatGPT.
### Notes
N/A
### What I Accomplished
I was able to set up the project with the relevant file structure and code given by ChatGPT. While trying to build the project, I ran into various bugs, mostly involving installing necessary CMake drivers and ESP version control. I am currently working with ChatGPT to resolve these errors.

## Thursday, May 22, 2025
### Task
Debug CMake errors that were appearing when trying to build the project; continue to debug in order for successful compilation to occur. 
### Notes
The device target confirmation in the Terminal does not show up unless initial build is successful. 
### What I Accomplished
I realized the CMake error was due to my version (5.1.1) not matching the overall version (5.4.1), so resolving this version conflict resolved the initial issue. However, there was a similar issue with the ESP-IDF version, though this was resolved in a similar manner and making sure all dependencies and filepaths were present and correct. I also fixed an issue with the single_image_detect.cpp referencing the wrong name of the single_image_detect.hpp file. However, a fatal error persisted in which pedestrian_detect.hpp was being referenced as a header in single_image_detect.hpp but was never created during build. 

## Friday, May 23, 2025
### Task
Find the cause as to why relevant files such as pedestrian_detect.hpp were not being pulled from the GitHub repo during build. 
### Notes
N/A
### What I Accomplished
I attempted lengthy debugging for this issue, mainly modifying CMakeLists.txt and idf.yml files. I managed to add an esp-dl folder into the project, which contained pedestrian_detect.hpp and other files listed as headers in the main project files. However, I realized the file paths listed were incorrect and that I needed to update those to reflect the current project file structure, as certain files were not being referenced and causing errors. 

## Monday, May 26, 2025
### Task
Fix file path and CMakeLists files to ensure the correct files were being used in file headers. 
### Notes
N/A
### What I Accomplished
I modified the CMakeLists file in the main folder to reference directories that included files mentioned in the detect_single_image.cpp headers. I also cleaned up the file structure a little bit - removing the main folder inside the pedestrian_detect folder - and updated the file paths referenced in all the CMakeLists files. I also ensured that each level of CMakeLists was referencing the correct SRC/private file directories. Currently, I'm repeating this process to make sure any files within the esp-idf-v5.1.1 folder are also referenced properly. 

## Tuesday, May 27, 2025
### Task
Make sure files in the esp-idf-v5.1.1 were referenced properly.
### Notes
N/A
### What I Accomplished
I found including the file path to esp_vfs_fat.h in main/CMakeLists was incorrect and was a version mismatch as the ESP-IDF version I was using was 5.4.1 and the path was meant for version 5.1.1. To fix this, I put fatfs under the REQUIRES header instead. This resolved the final filepath issue, leaving only compilation issues afterwards. The bulk of compilation issues seem to be coming from single_image_ped_detect.hpp and single_image_ped_detect.cpp, which were generated and thus likely don't entirely work with the existing file structure. My next steps will be to copy in the existing pedestrian_detect code and modify along the way as necessary for our purpose. 

## Wednesday, May 28, 2025
### Task
Resolve all compilation errors and generate code to detect pedestrians from an SD card. 
### Notes
Maintaining a simple file structure minimizes most version and compilation errors. 
### What I Accomplished
I cleaned up the project file structure in order for it to mirror the original pedestrian_detect file structure. This led to much of the version and file errors being resolved as the project was able to properly pull relevant files from the Espressif GitHub repository and generate proper build files. I utilized Gemini to generate sample code for detecting pedestrians using the SD card, which I hope to test soon with example images. 

## Friday, May 30, 2025
### Task
Debug code for detecting pedestrians from an SD card.
### Notes
N/A
### What I Accomplished
I downloaded the .zip file with the test images and loaded them onto the SD card. I then imported the contents of the SD card into my project and put the file path for one of the images in the generated code. However, I ran into compilation issues with the code, particularly regarding the JPEG decoder to convert the JPG image files to a format useful for the code. I tried debugging by ensuring the right file was referenced as a header - however, the compiler was not able to find this file in the system. I will do research and hopefully find videos detailing this functionality as there is a possibility this may not be compatible with the ESP32-S3 (the device we have). 

## Tuesday, June 3, 2025
### Task
Get the JPEG decoder example from Espressif to work with the device.
### Notes
It seems the JPEG decoder example is likely not supported anymore, as various build dependencies weren't being found and the issues were replicated with other devices. 
### What I Accomplished
I attempted to debug the runtime errors happening when the JPEG decoder example was running. I was told by my mentor to try setting a device target and installing Python extension requirements; however, this did not resolve the issue. I also tried changing the ESP-IDF extension configuration to use Espressif instead of GitHub, but this did not help, either. After being notified by my mentor that the script wasn't running on his end, either, we came to the conclusion that this example is likely not supported anymore with the current version of ESP-IDF. My mentor also found another example closer to our end goal - a version of pedestrian_detect that embeds an image in firmware, runs JPEG decoding, and prints out the locations of pedestrians in the image to the terminal. From here on out, I will work off of this script. 

## Wednesday, June 4, 2025
### Task
Begin working on adapting the new pedestrian_detect example to take an image from the SD card. 
### Notes
N/A
### What I Accomplished
I reconfigured the GitHub repository with the relevant files for the new pedestrian_detect example, though it did result in me needing to create a new one for that purpose. However, I was able to get the repository up and running. After this, I ran prompts to modify the existing code to pull images from an SD card rather than embedding it in the firmware; however, this led to many compilation issues. Moving forward, I plan to break down the code on my own and figure out what specific parts need to be changed, using existing YouTube videos as reference. 

## Thursday, June 5, 2025
### Task
Generate and debug code for pulling images from an SD card based on the new pedestrian_detect example. 
### Notes
It seems that the SD card images need to be properly sized in order for the program to work. Currently, the images are too large to be decoded. 
### What I Accomplished
I generated a few lines to change the functionality of the example code to pull images from the SD card. The program was able to build, flash, and monitor; however, I kept getting an error that displayed "Failed to allocate output buffer". After some research, I learned that this error was due to the program not being able to allocate the proper amount of memory to store the image after it's decoded. From here, I plan to manually resize the image to 240 x 280 pixels in Canva and upload it to the SD card and see if the issue is resolved. Afterward, I hope to create a resize function within the code for efficiency and functionality. 