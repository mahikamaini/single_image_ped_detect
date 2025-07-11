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
I generated a few lines to change the functionality of the example code to pull images from the SD card. The program was able to build, flash, and monitor; however, I kept getting an error that displayed "Failed to allocate output buffer". After some research, I learned that this error was due to the program not being able to allocate the proper amount of memory to store the image after it's decoded. I searched the ESP-IDF API for a method that performed this function; however, there isn't anything that matches. From here, I plan to manually resize the image to 240 x 240 pixels in Canva and upload it to the SD card and see if the issue is resolved. Afterward, I hope to create a resize function within the code for efficiency and functionality. 

## Thursday, June 12, 2025
### Task
Resize one test image to 240 x 240 pixels and see if the memory issue resolves and the model is able to detect a pedestrian. 
### Notes
The image does indeed need to be 240 x 240 pixels for enough memory to be allocated towards decoding. 
### What I Accomplished
I uploaded one of the test set images into Canva and resized it. However, after flashing it to the SD card and running the model on it, nothing was printed to the terminal - leaving me unsure whether the model was working or if it was simply because the pedestrian was not clearly visible in the image. After resizing and flashing another image with a clear view of the pedestrian, running the model resulted in a model prediction being printed to the terminal, indicating working success of the model on properly sized images. From here, I began to work on a function that would automatically resize the image to 240 x 240 pixels, although I've been running into numerous issues, especially with storage. 

## Friday, June 13, 2025
### Task
Continue work on a resizing function for the input JPEG images. 
### Notes
N/A
### What I Accomplished
While trying to write the function, I kept getting the "Failed to allocate output buffer" error, indicating the input JPEG image may be too large for the ESP32 to handle. I tried using grayscale images to save on memory; however, it turned out that the pedestrian detect model did not support grayscale as a valid image format. Upon further research, I learned about optimizing the board's external RAM (SPI RAM). I made sure to check the menuconfig settings and tried adding commands to the code to utilize this, but the original error still persisted. Moving forward, I will try to compress the test image beforehand to see if that helps to resolve the error and work from there. 

## Monday, June 16, 2025
### Task
Test out compressing the original image to see if the resize function works. 
### Notes
The original image must be a maximum of ~ 200 KB in order for it to be small enough for the resize function to work. However, this leads to a lower model prediction confidence. 
### What I Accomplished
Using an online image compressor, I compressed the original JPEG image to about 500 KB from 719 KB. However, error still persisted. I then compressed the image further to 204 KB, and the resize function was able to run and print out model predictions to the terminal. Shortly after, I added to the program so that the results were printed to a .txt file in the SD card instead of to the terminal. However, I noticed that the model confidence dropped down to 83% from 96% beforehand. This indicated to me that the compressing of the image decreased the quality of predictions. During our weekly meeting, Ben and I discussed creating a Python script in the local filesystem to reduce the tediousness of compressing and downloading images from external sites. 

In addition, we discussed implementing multi-image detection - in which the program and detection model runs on multiple images on the SD card. If we are able to get this feature working, we may be able to look at putting together a report to submit to the CamTrapAI Workshop Series by July 1st. 

## Tuesday, June 17, 2025
### Task
Incorporate a Python script to compress images into the project. 
### Notes
The original image size (2048x1536) results in a ~6kB RAW image when decoded, which takes up too much memory, which is why we're resizing the image to 240x240 before it gets to the model. We could potentially resize before decoding so that the amount of memory required is lessened and may lead to better predictions. 
### What I Accomplished
On an online resource, we found a Python script that compressed JPG images and allowed the user to input their desired compression quality, where a higher number indicated less compression. Copying this script, I created a file in my project and moved the test image to the same folder to test out the script. When compiling the script in Terminal, the test image was successfully compressed into a separate image file. After loading this compressed image onto the SD card, I was able to run the detection program with little changes - I only needed to release the image buffer and image data resources sooner in the program to conserve memory. However, the prediction confidence was still only about 82%, indicating that the model was substantially affected with file compression. Even after modifying the compression quality to see if there were any significant confidence prediction changes, I was only able to get the confidence up to about 84%. Looking forward, we are looking into configuring resizing into the decoder, so that the image is resized before it's decoded and thus eliminates any memory allocation issues. 

## Wednesday, June 18, 2025
### Task
Test out decoder resizing and note how it affects the model confidence. 
### Notes
N/A
### What I Accomplished
I tested out the decoder function that Ben had edited to specify the resulting image size (e.g. 2 == 1/2 original size). However, I found that this only took common factors - otherwise, the program would crash. With a factor of 4 (1/4 of the original image size), the model confidence was 76%, while a factor of 2 resulted in 83%. This was expected as a larger scale allows for more of the original image details to be preserved. After doing some further research, ChatGPT suggested utilizing an existing crop parameter in the resize() function to crop the center of the image and then resize it to reduce distortion. I incorporated this into the code and the confidence was 89%. After further discussion, though, Ben and I realized this would not work for all the images, as the pedestrian isn't always necessarily in the center of the image. From here, ChatGPT gave the idea to perform left and right crops if the center crop had an accuracy lower than a specified threshold or detected no pedestrians at all. 

## Friday, June 20, 2025
### Task
Implement left and right crops with the model. 
### Notes
N/A
### What I Accomplished
I implemented the left and right image crops with a threshold of 85%. This means that if the center crop has an accuracy lower than 85%, it will move on to the left crop, then the right crop. I tested this model on several images and I achieved confidence of mostly 81% and higher. Ben suggested to make each of the crops 240x240 pixels instead of cropping and then resizing the image to save on memory. I attempted this method; however, when testing on an image with a clear pedestrian in it, the model detected no pedestrians. From here, I moved on to implement running the program on multiple images in the SD card. However, it only seemed to be able to process 3 images or so before crashing, and wasn't an issue related to the file itself. I checked the memory logs and there seems to be plenty of PSRAM and SPIRAM to support the image, so the exact cause of this issue is unknown to me, other than that the resize() function failed on the fourth image. 

## Monday, June 23, 2025
### Task
Debug the resize() function failure.
### Notes
It seems that MacOS generates hidden files on SD cards that start with _. and end with .JPG. Thus, the program needs to have a check for these kinds of files so it will not attempt to read them. 
### What I Accomplished
I ran through various solutions to try and debug this function. Ben had told me the program worked perfectly with his PC, so the issue was likely with my local environment or SD card. I reformatted my SD card to FAT32 but this did not resolve the issue. I also tried recloning the GitHub repository locally but the same issue persisted. During our meeting, Ben suggested downgrading the ESP-IDF version from 5.4.1 to 5.4.0. However, after further research, ChatGPT suggested that MacOS may be generating hidden files on the SD card, so through Terminal, I looked and discovered that they did exist on the SD card. From this knowledge, I concluded the issue with the resize() function crashing was that the program was attempting to read these hidden files which had no content in them. With this, I added a condition to the program to not read any files that started with "_." - as this was the naming scheme for these hidden files. Moving forward, I plan to create a Python script to generate a CSV file that compares the model prediction with the results in terms of finding a pedestrian. This will allow me to create a truth table in the future. 

## Tuesday, June 24, 2025
### Task
Create a Python script to generate a CSV file comparing model predictions vs. actual results. 
### Notes
N/A
### What I Accomplished
I created a Python script within the project (compare_results.py) that goes through each of the subfolders of the test set and matches the name of each image path to the results on detection_results.txt. For this, I created a match() function that took two parameters - the path to detection_results.txt and the name of the image path. This function extracts the image name from each line of the file and matches it to the given image name. Then, depending on what the correct line reads, it assigns "FALSE" if it says "No pedestrian detected" and "TRUE" if it says "pedestrian(s) detected". Finally, these are all written to a CSV file called ground_truth.csv.

## Wednesday, June 25, 2025
### Task
Modify the printed x and y coordinates in the program to be relative to the original image rather than the resized image. 
### Notes
N/A
### What I Accomplished
I multiplied the original number by the crop_size variable divided by 240 in order to get the scale factor for the cropping. I then created a variable called crop_number to keep track of the crop performed (0 = center crop, 1 = left crop, 2 = right crop). From here, I used this variable to get to the right index in crop_areas (which is a two-dimensional vector). Each crop vector has four numbers: an x-offset, y-offset, width, and height. Depending on each coordinate, I either put index 0 (x-offset) or index 1 (y-offset). This value was added to the division before, and the entire equation multiplied by 2 as the original image was compressed by 1/2. 

## Monday, June 30, 2025
### Task
Troubleshoot the printed x and y coordinates to ensure they were accurate to the original image.  
### Notes
N/A
### What I Accomplished
After reviewing the coordinate formulas with Ben, we realized there wass no need to multiply the original number by crop_size / 240, or multiply it all by 2. Upon further inspection, we noted that the vectors in crop_areas already included the original size and crop_size as part of the x- and y-offsets for each kind of crop. Thus, we only needed to add the corresponding entry in crop_areas to the original res.box value. In addition, using the data from the smaller dataset, I created a confusion matrix in Excel. The next steps I will take is using a larger dataset to get a more accurate depiction of how the model performs on images, including images with animals in them. 

## Tuesday, July 1, 2025
### Task
Configure my existing program to download images from an HTTP server on my laptop rather than pulling from the SD card. 
### Notes
N/A
### What I Accomplished
After examining the larger dataset that Ben had provided for me, I realized that it was too large (~150 GB) to host on my SD card (32 GB). Through research, I determined that hosting an HTTP server locally and having the ESP32 download the images one by one to run the program on would be the simplest solution to this. I added code to my program to have the ESP32 connect to my local WiFi network and ensured that I opened the server from the command line in the image directory. From here, I used the command line to generate a text file with all the image names and saved it in the image directory. I ran into several errors while trying to test the code, and I was able to resolve some by adding the relevant dependent directories in main/CMakeLists.txt. However, I am continuously experiencing an error that the ESP32 wasn't able to find my laptop on my home WiFi. 

## Wednesday, July 2, 2025
### Task
Resolve the WiFi issue with the ESP32 and test the program. 
### Notes
N/A
### What I Accomplished
I realized my laptop was on a different WiFi network than the device, so I switched the network - however, the issue still persisted. I also ensured that no blocking was active on the network that would prevent the ESP32 from joining WiFi. After talking with Ben, I decided to test one of the STA WiFi examples provided by ESP-IDF. The device successfully ran this program and was able to connect to my WiFi, ruling out the possibility of a network issue. From here, I copied over the WiFi logic from the example program into my project and this resolved the WiFi issue. However, the program ran into an error that the resize resolution of the image wasn't a multiple of 8 as the original image sizes were bigger than the previous dataset. 

## Thursday, July 3, 2025
### Task
Solve the resize resolution issue and test program images. 
### Notes
Because these new images are larger than the previous dataset, we need to ensure their dimensions are divisible by 8 for the program to run. 
### What I Accomplished
I modified the resize() function to remove the scale factor parameter, and instead, added a loop in the code that cycled through scale factors from 2 to 8 until the dimensions of the image were divisible by 8. After making this change, the program ran fairly smoothly - however, at certain images, there seemed to be a memory leak and the image was unable to be processed. After some research, I decided a cleanup function may help with this issue; however, I am still working on implementing it. It seems these trouble images are ones that have animals in them, and the model detects them as humans. 

## Monday, July 7, 2025
### Task
Identify the runtime errors in the program and resolve them. 
### Notes
N/A
### What I Accomplished
After examining the logs during runtime, I concluded there were two types of errors: a memory allocation error and output buffer allocation error. The memory allocation error occured when the image file was too large (> 2.5 MB); however, the output buffer allocation error was strange and I was unsure of its cause. During our meeting, Ben and I moved the statement printing the amount of available SPIRAM earlier in the code to track memory fragmentation as a possible cause. However, neither of these errors directly resulted in this. As the program continued to run, though, I observed the available SPIRAM decreasing over time, which Ben suggested indicated an issue with downloading the images over the local server. My next steps with this will be to debug this process and seeing how it affects these errors. 

## Tuesday, July 8, 2025
### Task
Debug SPIRAM fragmentation issues.
### Notes
N/A
### What I Accomplished
As per research, I tried allocating a fixed-size buffer of 2.5 MB instead of a dynamic buffer for each image - however, this did not resolve the issue and instead led to more images not being processed. For memory efficiency purposes, I also tried incorporating an approach that directly cropped to 240x240 pixels without affecting the placement of objects in the image, but this caused runtime errors, as well. After searching through the menuconfig settings, I found a setting to "Free dynamic buffers during WiFi enterprise connection". I enabled this and noticed that while the initial available SPIRAM (~3.5 MB) persisted for longer, one image in the middle of processing caused this number to drop to ~ 2.4 MB. 

## Wednesday, July 9, 2025
### Task
Continue to debug SPIRAM fragmentation and produce .txt file of images. 
### Notes
N/A
### What I Accomplished
I tried decoding the image to grayscale to save on memory, then converting it to RGB888; however, since the Pedestrian Detect model does not take grayscale as a valid image format, this approach was unsuccessful. I defined a variable called MIN_FREE_SPIRAM to be ~ 2.5 MB and added a check that restarted the ESP32 if the amount of available SPIRAM dropped below this threshold. Finally, in order to save the last image index reached, I configured Non-Volatile Storage (NVS) - a built-in ESP32 library. This way, even when the device is disconnected or restarted, the program will continue from the saved index instead of from the beginning. 

## Thursday, July 10, 2025
### Task
Monitor program while it processes the 7000 images in the dataset. 
### Notes
N/A
### What I Accomplished
I noticed that the amount of available SPIRAM did slowly decrease, although the ESP32 only restarted about once or twice in total. However, the program crashes near the 6900th image because of an invalid image URL, then restarts from a different last known index - an extremely odd error. From here, I will put in a check for this error and finish running all the images in the dataset, before moving on to the camera phase of the project. 