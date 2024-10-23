#pragma once

#include "libraries/inky_frame_7/inky_frame_7.hpp"
#include "JPEGDEC.h"

class InkyFrameManager {
  public:
    enum Event {
      NONE,
      IMAGE_A,
      IMAGE_B,
      IMAGE_C,
      IMAGE_D,
      IMAGE_E,
      PRINT_IP_ADDRESS
    };

    void init(pimoroni::InkyFrame* inky);
    int update_image(int slot, std::string filename);
    void draw_image(int slot);
    void print_message(std::string message);
    InkyFrameManager::Event poll();

  private:
    struct ButtonState {
      uint32_t last_on_press_time_a;
      uint32_t last_on_press_time_b;
      uint32_t last_on_press_time_c;
      uint32_t last_on_press_time_d;
      uint32_t last_on_press_time_e;
    };

    pimoroni::InkyFrame* inky;
    JPEGDEC jpeg;   
    ButtonState button_state;

    int check_button_state(pimoroni::InkyFrame::Button button, uint32_t* button_state);
    void draw_image_task(std::string filename);
    void draw_image(std::string filename, pimoroni::InkyFrame::LED led);
    void blink_led_task(pimoroni::InkyFrame::LED led);
    void print_message_task(std::string message);

    static void start_draw_image_task(void* param);
    static void start_blink_led_task(void* param);
    static void start_print_message_task(void* param);
};