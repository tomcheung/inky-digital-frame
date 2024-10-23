#include "inky_frame_manager.hpp"

#include <memory>
#include <iostream>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

#define TEST_TASK_PRIORITY				( tskIDLE_PRIORITY + 1UL )
#define TEST_TASK_STACK_SIZE			(( configSTACK_DEPTH_TYPE ) 2048)

using namespace pimoroni;

void InkyFrameManager::init(InkyFrame* inky) {
  this->inky = inky;
}

struct jpeg_decode_data {
  int x, y, img_w, img_h, frame_w, frame_h;
  InkyFrame* inky;
};

struct draw_text_data {
  InkyFrameManager* instance;
  std::string message;
};

struct image_task_param {
  InkyFrameManager* instance;
  std::string filename;
  InkyFrame::LED led;
  TaskHandle_t led_task = NULL;
};

void* jpegdec_open_callback(const char *filename, int32_t *size) {
  FIL *fil = new FIL;
  if(f_open(fil, filename, FA_READ)) {
    std::cout << "Unable to read file" << std::endl;
    return nullptr;
  }
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
  jpeg_decode_data* data = (jpeg_decode_data*) draw->pUser;
  InkyFrame* inky = data->inky;

  uint16_t *p = draw->pPixels;

  int xo = data->x;
  int yo = data->y;

  for(int y = 0; y < draw->iHeight; y++) {
    for(int x = 0; x < draw->iWidth; x++) {
      int sx = ((draw->x + x + xo) * data->frame_w) / data->img_w;
      int sy = ((draw->y + y + yo) * data->frame_h) / data->img_h;

      if(xo + sx > 0 && xo + sx < inky->bounds.w && yo + sy > 0 && yo + sy < inky->bounds.h) {
        inky->set_pixel_dither({xo + sx, yo + sy}, RGB(RGB565(*p)));
      }

      p++;
    }
  }

  return 1; // continue drawing
}

// Draw to the nearest colour instead of dithering
int jpegdec_draw_posterize_callback(JPEGDRAW *draw) {
  jpeg_decode_data* data = (jpeg_decode_data*) draw->pUser;
  InkyFrame* inky = data->inky;

  uint16_t *p = draw->pPixels;

  int xo = data->x;
  int yo = data->y;

  for(int y = 0; y < draw->iHeight; y++) {
    for(int x = 0; x < draw->iWidth; x++) {
      int sx = ((draw->x + x + xo) * data->frame_w) / data->img_w;
      int sy = ((draw->y + y + yo) * data->frame_h) / data->img_h;

      if(xo + sx > 0 && xo + sx < inky->bounds.w && yo + sy > 0 && yo + sy < inky->bounds.h) {
        int closest = RGB(RGB565(*p)).closest(inky->palette, inky->palette_size);
        if (closest != -1) {
          inky->set_pen(closest);
          inky->set_pixel({xo + sx, yo + sy});
        } else {
          inky->set_pen(0);
          inky->set_pixel({xo + sx, yo + sy});
        }
      }

      p++;
    }
  }

  return 1; // continue drawing
}

InkyFrameManager::Event InkyFrameManager::poll() {
  uint32_t t_ms = to_ms_since_boot(get_absolute_time());

  int buttons_state[5] = { };

  buttons_state[0] = this->check_button_state(InkyFrame::BUTTON_A, &button_state.last_on_press_time_a);
  buttons_state[1] = this->check_button_state(InkyFrame::BUTTON_B, &button_state.last_on_press_time_b);
  buttons_state[2] = this->check_button_state(InkyFrame::BUTTON_C, &button_state.last_on_press_time_c);
  buttons_state[3] = this->check_button_state(InkyFrame::BUTTON_D, &button_state.last_on_press_time_d);
  buttons_state[4] = this->check_button_state(InkyFrame::BUTTON_E, &button_state.last_on_press_time_e);

  // DEBUG ONLY
  if (buttons_state[0] != 0 || buttons_state[1] != 0 || buttons_state[2] != 0 || buttons_state[3] != 0 || buttons_state[4] != 0) {
    std::cout << "Btn state: ";

    for (int v : buttons_state) {
      std::cout << v << " | ";
    }

    std::cout << std::endl;
  }
  // -----

  if (buttons_state[4] > 800) {
    return InkyFrameManager::Event::PRINT_IP_ADDRESS;
  }

  if (buttons_state[0] > 10) {
    return InkyFrameManager::Event::IMAGE_A;
  }

  if (buttons_state[1] > 10) {
    return InkyFrameManager::Event::IMAGE_B;
  }

  if (buttons_state[2] > 10) {
    return InkyFrameManager::Event::IMAGE_C;
  }

  if (buttons_state[3] > 10) {
    return InkyFrameManager::Event::IMAGE_D;
  }

  if (buttons_state[4] > 10) {
    return InkyFrameManager::Event::IMAGE_E;
  }

  return InkyFrameManager::Event::NONE;
}

int InkyFrameManager::check_button_state(pimoroni::InkyFrame::Button button, uint32_t* button_last_press_time) {
  uint32_t t_ms = to_ms_since_boot(get_absolute_time());
  bool is_pressed = inky->pressed(button);

  if (is_pressed && *button_last_press_time == 0) {
    *button_last_press_time = t_ms;
    return -1;
  } else if (!is_pressed && *button_last_press_time != 0) {
    int dt = t_ms - *button_last_press_time;
    *button_last_press_time = 0;
    return dt;
  }

  return 0;
}

void InkyFrameManager::blink_led_task(InkyFrame::LED led) {
  while (true) {
    inky->led(led, 100);
    vTaskDelay(500/portTICK_PERIOD_MS);
    inky->led(led, 0);
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

void InkyFrameManager::draw_image(int slot) {
  auto led = InkyFrame::LED::LED_A;
  switch (slot) {
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
  default:
    led = InkyFrame::LED::LED_A;
    break;
  }

  std::string image_name = "image_";
  image_name.append(std::to_string(slot));
  image_name.append(".jpg");

  draw_image(image_name, led);
}

void InkyFrameManager::draw_image(std::string filename, InkyFrame::LED led) {
  printf("start_draw_image_task\n");
  image_task_param* task_param = new image_task_param;
  task_param->instance = this;
  task_param->led = led;
  task_param->filename = filename;

  xTaskCreate(
    InkyFrameManager::start_draw_image_task,
    "DrawImageTask",
    TEST_TASK_STACK_SIZE,
    task_param,
    TEST_TASK_PRIORITY,
    nullptr
  );

  xTaskCreate(
    InkyFrameManager::start_blink_led_task,
    "BlinkLed",
    100,
    task_param,
    TEST_TASK_PRIORITY,
    &task_param->led_task
  );
}

void InkyFrameManager::print_message(std::string message) {
  draw_text_data* task_param = new draw_text_data;
  task_param->instance = this;
  task_param->message = message;

  std::cout << "print_message" << message << std::endl;

  xTaskCreate(
    InkyFrameManager::start_print_message_task,
    "PrintMessage",
    TEST_TASK_STACK_SIZE,
    task_param,
    TEST_TASK_PRIORITY,
    nullptr
  );
}

void InkyFrameManager::print_message_task(std::string message) {
  inky->set_pen(InkyFrame::WHITE);
  inky->clear();

  inky->set_font("sans");
  inky->set_thickness(3);
  inky->set_pen(InkyFrame::BLACK);
  int32_t tw = inky->measure_text(message, 2);
  inky->text(message, {(inky->width / 2) - (tw / 2), inky->height / 2}, 2);
  inky->update();
}

void InkyFrameManager::draw_image_task(std::string filename) {
  std::cout << "Drawing image: " << filename << std::endl;

  jpeg_decode_data* decode_data = new jpeg_decode_data();

  decode_data->x = 0;
  decode_data->y = 0;
  decode_data->frame_w = this->inky->width;
  decode_data->frame_h = this->inky->height;
  decode_data->inky = this->inky;

  int result = jpeg.open(
    filename.c_str(),
    jpegdec_open_callback,
    jpegdec_close_callback,
    jpegdec_read_callback,
    jpegdec_seek_callback,
    jpegdec_draw_callback // Try jpegdec_draw_posterize_callback
  );

  decode_data->img_w = jpeg.getWidth();
  decode_data->img_h = jpeg.getHeight();

  jpeg.setPixelType(RGB565_BIG_ENDIAN);
  jpeg.setUserPointer(decode_data);

  printf("- starting jpeg decode..");
  int start = millis();
  jpeg.decode(0, 0, 0);
  printf("done in %d ms!\n", int(millis() - start));

  std::cout << "End Drawing image: " << result << std::endl;
  jpeg.close();

  inky->update();
  delete decode_data;
}

int InkyFrameManager::update_image(int slot, std::string filename) {
  if (slot < 0 || slot > 4) {
    return 10;
  }
  jpeg_decode_data* decode_data = new jpeg_decode_data();
  jpeg.open(
    "upload.tmp",
    jpegdec_open_callback,
    jpegdec_close_callback,
    jpegdec_read_callback,
    jpegdec_seek_callback,
    nullptr // Try jpegdec_draw_posterize_callback
  );
  jpeg.setUserPointer(decode_data);

  int w = jpeg.getWidth(), h = jpeg.getHeight(), error = jpeg.getLastError();

  std::cout << "Uploading image width:" << w << " hegiht:" << h << "error code:" << error << std::endl;
  if (error != 0) {
    return error;
  }

  if (w > inky->bounds.w || h > inky->bounds.h) {
    return 11;
  }

  std::string new_filename = "image_";
  new_filename.append(std::to_string(slot));
  new_filename.append(".jpg");

  f_unlink(new_filename.c_str()); // Remove old image
  int move_result = f_rename(filename.c_str(), new_filename.c_str());

  std::cout << "Move file" << filename << "->" << new_filename << std::endl;

  this->draw_image(slot);

  return 0;
}

void InkyFrameManager::start_draw_image_task(void* param) {
  image_task_param* task_param = static_cast<image_task_param*>(param);
  task_param->instance->draw_image_task(task_param->filename);

  vTaskDelete(task_param->led_task);
  delete task_param;

  vTaskDelete(nullptr);
}

void InkyFrameManager::start_blink_led_task(void* param) {
  image_task_param* task_param = static_cast<image_task_param*>(param);
  task_param->instance->blink_led_task(task_param->led);
  vTaskDelete(nullptr);
}

void InkyFrameManager::start_print_message_task(void* param) {
  draw_text_data* task_param = static_cast<draw_text_data*>(param);
  task_param->instance->print_message_task(task_param->message);

  delete task_param;
  vTaskDelete(nullptr);
}