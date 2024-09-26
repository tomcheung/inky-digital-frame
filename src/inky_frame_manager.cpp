#include "inky_frame_manager.hpp"

#include <memory>
#include <iostream>
#include "pico/stdlib.h"
using namespace pimoroni;

void InkyFrameManager::init(InkyFrame* inky) {
  this->inky = inky;
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