#pragma once

#include "libraries/inky_frame_7/inky_frame_7.hpp"

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
    int check_button_state(pimoroni::InkyFrame::Button button, uint32_t* button_state);
    ButtonState button_state;
};