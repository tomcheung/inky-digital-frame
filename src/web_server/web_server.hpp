#pragma once
#include <string>
#include <functional>

#include "mg_wrapper.hpp"

class WebServer {
  public:
    enum Event {
      none,
      upload_image
    };

    struct Message {
      WebServer::Event event;
      int new_image_slot;
    };

    WebServer(MgWrapper* mgr_wrapper);
    void start_server();
    void stop_server();
    void poll_data();
    int connect_wifi();
    std::string get_ip_address();
    WebServer::Message get_message();
    

  private:
    MgWrapper* mg_wrapper;
    WebServer::Message msg;
    void clear_message();

    static void eventHandler(struct mg_connection *c, int ev, void *ev_data);
};