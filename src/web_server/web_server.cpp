#include "web_server.hpp"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "FreeRTOS.h"
#include "task.h"
#include "mongoose.h"

#include "wifi.hpp"

struct MgWrapper {
  mg_mgr mgr;
};

WebServer::WebServer() {
  mg_wrapper = new MgWrapper{};
}

WebServer::~WebServer() {
  delete mg_wrapper;
}

void WebServer::start_server() {
  mg_log_set(MG_LL_DEBUG);
  auto mgr = &mg_wrapper->mgr;
  mg_mgr_init(mgr);
  mg_http_listen(mgr, "http://0.0.0.0:80", eventHandler, NULL); // Web listener
}

void WebServer::poll_data() {
  mg_mgr_poll(&mg_wrapper->mgr, 10);
  vTaskDelay(30/portTICK_PERIOD_MS);
}

void WebServer::eventHandler(struct mg_connection *c, int ev, void *ev_data) {
  if (ev == MG_EV_HTTP_MSG) {  // New HTTP request received
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;  // Parsed HTTP request
    if (mg_match(hm->uri, mg_str("/api/hello"), NULL)) {              // REST API call?
      mg_http_reply(c, 200, "", "{{%m:%d}\n", MG_ESC("status"), MG_ESC("ok"));    // Yes. Respond JSON
    } else {
      struct mg_http_serve_opts opts = {
        .root_dir = "/www_root",
        .fs = &mg_fs_packed
      };

      mg_http_serve_dir(c, hm, &opts);
    }
  }
}

int WebServer::connect_wifi() {
  printf("Connect wifi...\n");

  cyw43_arch_enable_sta_mode();
  
  int code = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);

  if (code) {
    printf("Failed to connect! code: %i\n", code);
  } else {
    auto ip_addr = cyw43_state.netif[CYW43_ITF_STA].ip_addr.addr;
    printf("Connected. IP Address: %lu.%lu.%lu.%lu\n", ip_addr & 0xFF, (ip_addr >> 8) & 0xFF, (ip_addr >> 16) & 0xFF, ip_addr >> 24);
  }

  return code;
}