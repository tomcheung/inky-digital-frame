#pragma once
#include <string>

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

    WebServer();
    ~WebServer();
    void start_server();
    void stop_server();
    void poll_data();
    int connect_wifi();
    std::string get_ip_address();
    WebServer::Message get_message();

  private:
    struct MgWrapper;
    MgWrapper* mg_wrapper;
    WebServer::Message msg;
    void clear_message();

    static void eventHandler(struct mg_connection *c, int ev, void *ev_data);
};