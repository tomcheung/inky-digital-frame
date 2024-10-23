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
          
          inky_manager.update_image(image_slot, "upload.tmp");
          f_close(fil);
        }
        case WebServer::Event::none:
          break;
      }
    }

    if (event == WebServer::Event::none) {
      auto event = inky_manager.poll();

      switch (event) {
        case InkyFrameManager::Event::IMAGE_A:
          inky_manager.draw_image(0);
          break;
        case InkyFrameManager::Event::IMAGE_B:
          inky_manager.draw_image(1);
          break;
        case InkyFrameManager::Event::IMAGE_C:
          inky_manager.draw_image(2);
          break;
        case InkyFrameManager::Event::IMAGE_D:
          inky_manager.draw_image(3);
          break;
        case InkyFrameManager::Event::IMAGE_E:
          inky_manager.draw_image(4);
          break;
        case InkyFrameManager::Event::PRINT_IP_ADDRESS:
        {
          std::string message = "IP:";
          message.append(server.get_ip_address());
          inky_manager.print_message(message);
          break;
        }
         
        case InkyFrameManager::Event::NONE:
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