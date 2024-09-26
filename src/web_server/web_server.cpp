#include "web_server.hpp"
#include <stdio.h>
#include <string.h>
#include <sstream> 
#include <iostream>
#include <memory>

#include "drivers/fatfs/ff.h"
#include "pico/cyw43_arch.h"

#include "FreeRTOS.h"
#include "task.h"
#include "mongoose.h"

struct WebServer::MgWrapper {
  mg_mgr mgr;
};

WebServer::WebServer(): mg_wrapper(new MgWrapper) {
  auto mgr = &mg_wrapper->mgr;
  msg = {WebServer::Event::none, -1};
}

void WebServer::start_server() {
  mg_log_set(MG_LL_INFO);
  auto mgr = &mg_wrapper->mgr;

  printf("start_server\n");
  mg_mgr_init(mgr);
  mg_http_listen(mgr, "http://0.0.0.0:80", WebServer::eventHandler, NULL); // Web listener
  mgr->userdata = this;
}

void WebServer::stop_server() {
  auto mgr = mg_wrapper->mgr;
  printf("stop_server\n");
  mg_mgr_free(&mgr);
  delete mg_wrapper;
}

void WebServer::poll_data() {
  mg_mgr_poll(&mg_wrapper->mgr, 50);
}

WebServer::Message WebServer::get_message() {
  auto message = msg;
  if(msg.event != WebServer::Event::none) {
    clear_message();
  }

  return message;
}

void WebServer::clear_message() {
  msg = {WebServer::Event::none, -1};
}

struct upload_state {
  size_t expected;  // POST data length, bytes
  size_t received;  // Already received bytes
  int image_index;
  void *fp;         // Opened uploaded file
};

void WebServer::eventHandler(struct mg_connection *c, int ev, void *ev_data) {
  struct upload_state *us = (struct upload_state *) c->data;
  struct mg_fs *fs = &mg_fs_fat;

  WebServer* server = static_cast<WebServer*>(c->mgr->userdata);
  
  // Catch /upload requests early, without buffering whole body
  // When we receive MG_EV_HTTP_HDRS event, that means we've received all
  // HTTP headers but not necessarily full HTTP body
  if (ev == MG_EV_HTTP_HDRS) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    if (mg_match(hm->uri, mg_str("/upload_image"), NULL)) {
      struct mg_str slot = mg_http_var(hm->query, mg_str("slot"));
      int index = 0;
      mg_str_to_num(slot, 10, &index, sizeof(index));
      printf("Uploading image to slot %i\n", index);
      char path[MG_PATH_MAX] = "upload.tmp";
      // mg_snprintf(path, sizeof(path), "%s/%.*s", "/tmp", hm->uri.len - 8,
      //             hm->uri.buf + 8);
      us->image_index = index;
      us->expected = hm->body.len;  // Store number of bytes we expect
      
      mg_iobuf_del(&c->recv, 0, hm->head.len);  // Delete HTTP headers
      c->pfn = NULL;  // Silence HTTP protocol handler, we'll use MG_EV_READ
      if (mg_path_is_sane(mg_str_s(path))) {
        fs->rm(path);                        // Delete file if it exists
        us->fp = fs->op(path, MG_FS_WRITE);  // Open file for writing
      }
    }
  }

  // Catch uploaded file data for both MG_EV_READ and MG_EV_HTTP_HDRS
  if (us->expected > 0 && c->recv.len > 0) {
    us->received += c->recv.len;
    if (us->fp) fs->wr(us->fp, c->recv.buf, c->recv.len);  // Write to file
    c->recv.len = 0;  // Delete received data
    //  MG_INFO(("Uploaded %lu / %lu bytes", us->received, us->expected));

    if (us->received >= us->expected) {
      // Uploaded everything. Send response back
      MG_INFO(("Uploaded finish %lu bytes", us->received));
      mg_http_reply(c, 200, NULL, "%lu ok\n", us->received);
      if (us->fp) fs->cl(us->fp);  // Close file

      server->msg = {
        WebServer::Event::upload_image,
        us->image_index
      };

      memset(us, 0, sizeof(*us));  // Cleanup upload state

      c->is_draining = 1;          // Close connection when response gets sent
    }
  }

  if (c->pfn == NULL) {
    return;
  }

  if (ev == MG_EV_HTTP_MSG) {  // New HTTP request received
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;  // Parsed HTTP request
    auto uri = hm->uri;
    if (mg_match(uri, mg_str("/set_message"), NULL)) {              // REST API call?
      char* msg = mg_json_get_str(hm->body, "$.message");
      printf("message len %i\n", strlen(msg));
     
      auto message = std::string(msg);

      mg_http_reply(c, 200, "", "{{%m:%s}\n", MG_ESC("status"), MG_ESC("ok"));    // Yes. Respond JSON
      
      // server->msg = message;
    } else {
      struct mg_http_serve_opts opts = {
        .root_dir = "/dist",
        .fs = &mg_fs_packed
      };
      
      mg_http_serve_dir(c, hm, &opts);
    }
  }
}

int WebServer::connect_wifi() {
  printf("Connect wifi...\n");

  cyw43_arch_enable_sta_mode();

  std::unique_ptr<FIL> fils(new FIL);
  FRESULT result = f_open(fils.get(), "wifi.txt", FA_READ);

  int code = -1;

  #if defined(WIFI_SSID) && defined(WIFI_PASSWORD)
   code = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);
  #else
   std::cout << "Cannot find WIFI_SSID or WIFI_PASSWORD config, read from wifi.txt" << std::endl;
    if (result == FR_OK) {
      char buffer[50] = {};
      f_gets(buffer, 50, fils.get());
      std::string ssid = std::string(buffer);

      f_gets(buffer, 50, fils.get());
      std::string password = std::string(buffer);

      std::cout << "Try to connect with SSID: " << ssid << std::endl;

      code = cyw43_arch_wifi_connect_timeout_ms(ssid.c_str(), password.c_str(), CYW43_AUTH_WPA2_AES_PSK, 30000);
    } else {
      std::cout << "Unable to read wifi.txt, code: " << result;
    }

    f_close(fils.get());
  #endif

  if (code) {
    printf("Failed to connect! code: %i\n", code);
  } else {
    auto ip_addr = cyw43_state.netif[CYW43_ITF_STA].ip_addr.addr;
    std::cout << "Connected. IP Address: " << get_ip_address() << std::endl;
  }

  return code;
}

std::string WebServer::get_ip_address() {
   auto ip_addr = cyw43_state.netif[CYW43_ITF_STA].ip_addr.addr;
   std::ostringstream ss;
   ss << (ip_addr & 0xFF) << "." << ((ip_addr >> 8) & 0xFF) << "." << ((ip_addr >> 16) & 0xFF)  << "." <<  (ip_addr >> 24);
   return ss.str();
}