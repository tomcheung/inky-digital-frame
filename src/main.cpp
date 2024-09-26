#include <string>
#include <iostream>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "hardware/vreg.h"

#include "JPEGDEC.h"

#include "libraries/pico_graphics/pico_graphics.hpp"
#include "libraries/inky_frame_7/inky_frame_7.hpp"

#include "FreeRTOS.h"
#include "task.h"

#include "inky_frame_manager.hpp"
#include "web_server.hpp"

#define TEST_TASK_PRIORITY				( tskIDLE_PRIORITY + 1UL )
#define TEST_TASK_STACK_SIZE			(( configSTACK_DEPTH_TYPE ) 2048)

using namespace pimoroni;

FATFS fs;

InkyFrame inky;

JPEGDEC jpeg;

WebServer server;
InkyFrameManager inky_manager;

struct {
  int x, y, w, h;
} jpeg_decode_options;

void *jpegdec_open_callback(const char *filename, int32_t *size) {
  FIL *fil = new FIL;
  if(f_open(fil, filename, FA_READ)) {return nullptr;}
  *size = f_size(fil);
  return (void *)fil;
}

void jpegdec_close_callback(void *handle) {
  f_close((FIL *)handle);
  delete (FIL *)handle;
}

int32_t jpegdec_read_callback(JPEGFILE *jpeg, uint8_t *p, int32_t c) {
  uint br; f_read((FIL *)jpeg->fHandle, p, c, &br);
  return br;
}

int32_t jpegdec_seek_callback(JPEGFILE *jpeg, int32_t p) {
  return f_lseek((FIL *)jpeg->fHandle, p) == FR_OK ? 1 : 0;
}

int jpegdec_draw_callback(JPEGDRAW *draw) {
  uint16_t *p = draw->pPixels;

  int xo = jpeg_decode_options.x;
  int yo = jpeg_decode_options.y;

  for(int y = 0; y < draw->iHeight; y++) {
    for(int x = 0; x < draw->iWidth; x++) {
      int sx = ((draw->x + x + xo) * jpeg_decode_options.w) / jpeg.getWidth();
      int sy = ((draw->y + y + yo) * jpeg_decode_options.h) / jpeg.getHeight();

      if(xo + sx > 0 && xo + sx < inky.bounds.w && yo + sy > 0 && yo + sy < inky.bounds.h) {
        inky.set_pixel_dither({xo + sx, yo + sy}, RGB(RGB565(*p)));
      }

      p++;
    }
  }

  return 1; // continue drawing
}

// Draw to the nearest colour instead of dithering
int jpegdec_draw_posterize_callback(JPEGDRAW *draw) {
  uint16_t *p = draw->pPixels;

  int xo = jpeg_decode_options.x;
  int yo = jpeg_decode_options.y;

  for(int y = 0; y < draw->iHeight; y++) {
    for(int x = 0; x < draw->iWidth; x++) {
      int sx = ((draw->x + x + xo) * jpeg_decode_options.w) / jpeg.getWidth();
      int sy = ((draw->y + y + yo) * jpeg_decode_options.h) / jpeg.getHeight();

      if(xo + sx > 0 && xo + sx < inky.bounds.w && yo + sy > 0 && yo + sy < inky.bounds.h) {
        int closest = RGB(RGB565(*p)).closest(inky.palette, inky.palette_size);
        if (closest != -1) {
          inky.set_pen(closest);
          inky.set_pixel({xo + sx, yo + sy});
        } else {
          inky.set_pen(0);
          inky.set_pixel({xo + sx, yo + sy});
        }
      }

      p++;
    }
  }

  return 1; // continue drawing
}

void draw_jpeg(std::string filename, int x, int y, int w, int h) {

  // TODO: this is a horrible way to do it but we need to pass some parameters
  // into the jpegdec_draw_callback() method somehow and the library isn't
  // setup to allow any sort of user data to be passed around - yuck
  jpeg_decode_options.x = x;
  jpeg_decode_options.y = y;
  jpeg_decode_options.w = w;
  jpeg_decode_options.h = h;

  jpeg.open(
    filename.c_str(),
    jpegdec_open_callback,
    jpegdec_close_callback,
    jpegdec_read_callback,
    jpegdec_seek_callback,
    jpegdec_draw_callback // Try jpegdec_draw_posterize_callback
  );

  jpeg.setPixelType(RGB565_BIG_ENDIAN);

  printf("- starting jpeg decode..");
  int start = millis();
  jpeg.decode(0, 0, 0);
  printf("done in %d ms!\n", int(millis() - start));


  jpeg.close();
}

void blink_led(void * pvParameters) {
  InkyFrame::LED* p_led = (InkyFrame::LED*) pvParameters;
  InkyFrame::LED led = *p_led;

  while (true) {
    inky.led(led, 100);
    vTaskDelay(500/portTICK_PERIOD_MS);
    inky.led(led, 0);
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

TaskHandle_t paint_task = NULL;
TaskHandle_t paint_led_task = NULL;

void paint_image(void * pvParameters) {
  std::string* p = (std::string*) pvParameters;
  std::string file = std::string(*p);

  std::cout << "Displaying image: " << file << std::endl;

  draw_jpeg(file, 0, 0, inky.width, inky.height);
  inky.update();

  printf("Finish paint\n");

  if(paint_led_task != NULL) {
    vTaskDelete(paint_led_task);
    inky.led(InkyFrame::LED_A, 0);
    inky.led(InkyFrame::LED_B, 0);
    inky.led(InkyFrame::LED_C, 0);
    inky.led(InkyFrame::LED_D, 0);
    inky.led(InkyFrame::LED_E, 0);
  } 
  
  paint_task = NULL;
  vTaskDelete(NULL);
}

void print_image(std::string image_name, InkyFrame::LED led) {
  if (paint_task != NULL) {
    vTaskDelete(paint_task);
    vTaskDelete(paint_led_task);
  }
  xTaskCreate(blink_led, "PaintLed", 256, (void *) &led, TEST_TASK_PRIORITY, &paint_led_task);
  xTaskCreate(paint_image, "Paint", TEST_TASK_STACK_SIZE, (void *) &image_name, TEST_TASK_PRIORITY, &paint_task);
}

void print_ip_address_task(void * pvParameters) {
  std::string* msg_ptr = (std::string*) pvParameters;
  std::string message = "IP:";
  message.append(server.get_ip_address());

  inky.set_pen(InkyFrame::WHITE);
  inky.clear();

  inky.set_font("sans");
  inky.set_thickness(3);
  inky.set_pen(InkyFrame::BLACK);
  int32_t tw = inky.measure_text(message, 2);
  inky.text(message, {(inky.width / 2) - (tw / 2), inky.height / 2}, 2);
  inky.update();

  paint_task = NULL;
  vTaskDelete(NULL);
}

void print_ip_address() {
  if (paint_task != NULL) {
    vTaskDelete(paint_task);
  }
  xTaskCreate(print_ip_address_task, "PaintMessage", TEST_TASK_STACK_SIZE, NULL, TEST_TASK_PRIORITY, &paint_task);
}

void main_task(__unused void *params) {
  stdio_init_all();
  cyw43_arch_init();
  inky.init();

  inky_manager.init(&inky);

  vTaskDelay(100/portTICK_PERIOD_MS);

  printf("Init...\n");

  TaskHandle_t ledTask = NULL;

  auto led = InkyFrame::LED::LED_CONNECTION;
  xTaskCreate(blink_led, "LED", 256, (void *) &led, TEST_TASK_PRIORITY, &ledTask);

  printf("mounting sd card.. ");

  FRESULT fr;
  fr = f_mount(&fs, "", 1);
  if (fr != FR_OK) {
    printf("Failed to mount SD card, error: %d\n", fr);

    vTaskDelete(ledTask);
    vTaskDelete(NULL);
  }

  printf("done!\n");

  printf("connect wifi\n");

  int wifi_result = server.connect_wifi();

  vTaskDelete(ledTask);

  if (wifi_result == 0) {
    inky.led(InkyFrame::LED::LED_CONNECTION, 100);
    server.start_server();
  } else {
    inky.led(InkyFrame::LED::LED_CONNECTION, 0);
  }

  while (true) {
    WebServer::Event event = WebServer::Event::none;
    if (wifi_result == 0) {
      server.poll_data();

      WebServer::Message msg = server.get_message();
      event = msg.event;
      auto led = InkyFrame::LED::LED_ACTIVITY;

      switch (event) {
        case WebServer::Event::upload_image: {
          int image_slot = msg.new_image_slot;

          printf("upload_image %i\n", image_slot);
          if (image_slot<0 || image_slot >4) {
            printf("Invalid slot\n");
            break;
          }

          FIL *fil = new FIL;
          if(f_open(fil, "upload.tmp", FA_READ)) { 
            printf("Upload file missing\n");
            break;
          }

          jpeg.open(
            "upload.tmp",
            jpegdec_open_callback,
            jpegdec_close_callback,
            jpegdec_read_callback,
            jpegdec_seek_callback,
            nullptr // Try jpegdec_draw_posterize_callback
          );

          int w = jpeg.getWidth(), h = jpeg.getHeight(), error = jpeg.getLastError();

          char image_name[15];

          sprintf(image_name, "image_%i.jpg", image_slot); 

          printf("Uploading image width %i, hegiht %i, error %i\n", w, h, error);

          if (w>0 && h>0 && error == 0) {
            f_unlink(image_name);
            auto result = f_rename("upload.tmp", image_name);
            printf("Rename to %s result: %i\n", image_name, result);
          } else {
            f_unlink("upload.tmp");
          }

          f_close(fil);

          switch (image_slot) {
          case 0:
            led = InkyFrame::LED::LED_A;
            break;
          case 1:
            led = InkyFrame::LED::LED_B;
            break;
          case 2:
            led = InkyFrame::LED::LED_C;
            break;
          case 3:
            led = InkyFrame::LED::LED_D;
            break;
          case 4:
            led = InkyFrame::LED::LED_E;
            break;
          }

          auto name = std::string(image_name);
          if (!name.empty() && paint_task == NULL) {
            xTaskCreate(blink_led, "PaintLed", 256, (void *) &led, TEST_TASK_PRIORITY, &paint_led_task);
            xTaskCreate(paint_image, "Paint", TEST_TASK_STACK_SIZE, (void *) &name, TEST_TASK_PRIORITY, &paint_task);
          }
          break;
        }
        case WebServer::Event::none:
          break;
      }
    }

    if (event == WebServer::Event::none) {
      auto event = inky_manager.poll();

      switch (event) {
        case InkyFrameManager::Event::IMAGE_A:
          print_image("image_0.jpg", InkyFrame::LED::LED_A);
          break;
        case InkyFrameManager::Event::IMAGE_B:
          print_image("image_1.jpg", InkyFrame::LED::LED_B);
          break;
        case InkyFrameManager::Event::IMAGE_C:
          print_image("image_2.jpg", InkyFrame::LED::LED_C);
          break;
        case InkyFrameManager::Event::IMAGE_D:
          print_image("image_3.jpg", InkyFrame::LED::LED_D);
          break;
        case InkyFrameManager::Event::IMAGE_E:
          print_image("image_4.jpg", InkyFrame::LED::LED_E);
          break;
        case InkyFrameManager::Event::PRINT_IP_ADDRESS:
          print_ip_address();
          break;
      }
    }
  }

  inky.sleep();
}

void vLaunch(void) {
  TaskHandle_t task;
  xTaskCreate(main_task, "TestMainThread", TEST_TASK_STACK_SIZE, NULL, TEST_TASK_PRIORITY, &task);
  vTaskStartScheduler();
}

int main(void) {
  stdio_init_all();
  sleep_ms(1000);
  vLaunch();
  return 0;
}