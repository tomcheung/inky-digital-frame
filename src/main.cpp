#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "hardware/vreg.h"

#include "JPEGDEC.h"

#include "libraries/pico_graphics/pico_graphics.hpp"
#include "libraries/inky_frame_7/inky_frame_7.hpp"

#include "FreeRTOS.h"
#include "task.h"

#include "web_server.hpp"

#define TEST_TASK_PRIORITY				( tskIDLE_PRIORITY + 1UL )
#define TEST_TASK_STACK_SIZE			(( configSTACK_DEPTH_TYPE ) 2048)

using namespace pimoroni;

FATFS fs;
FRESULT fr;

InkyFrame inky;

JPEGDEC jpeg;

WebServer server;

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
  InkyFrame::LED* led = (InkyFrame::LED*) pvParameters;
  while (true) {
   inky.led(*led, 100);
   vTaskDelay(500/portTICK_PERIOD_MS);
   inky.led(*led, 0);
   vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

void web_server_task(__unused void *params) {
  while (true) {
    server.poll_data();
  }
}

void main_task(__unused void *params) {
  stdio_init_all();
  cyw43_arch_init();
  inky.init();

  vTaskDelay(100/portTICK_PERIOD_MS);

  printf("Init...\n");

  server = WebServer();

  TaskHandle_t ledTask = NULL;

  auto led = InkyFrame::LED::LED_CONNECTION;
  xTaskCreate(blink_led, "LED", 256, (void *) &led, TEST_TASK_PRIORITY, &ledTask);

  server.connect_wifi();

  printf("mounting sd card.. ");
  fr = f_mount(&fs, "", 1);
  if (fr != FR_OK) {
    printf("Failed to mount SD card, error: %d\n", fr);

    vTaskDelete(ledTask);
    vTaskDelete(NULL);
  }
  printf("done!\n");

  vTaskDelete(ledTask);
  server.start_server();

  xTaskCreate(web_server_task, "SERVER", TEST_TASK_STACK_SIZE, (void *) &server, TEST_TASK_PRIORITY, NULL);

  // inky.set_pen(InkyFrame::Pen::WHITE);
  // inky.clear();

  // printf("Displaying file: %s\n", "jwst1.jpg");
  // inky.set_pen(InkyFrame::Pen::BLACK);
  // draw_jpeg("jwst1.jpg", 0, 0, inky.width, inky.height);

  // inky.set_font("sans");
  // inky.text("Inky frame", Point(13, 10), 0, 1.0f);

  // inky.update();
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