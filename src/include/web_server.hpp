#pragma once

struct MgWrapper;

class WebServer {
  public:
    WebServer();
    ~WebServer();
    void start_server();
    void poll_data();
    int connect_wifi();

  private:
    MgWrapper* mg_wrapper;
    static void eventHandler(struct mg_connection *c, int ev, void *ev_data);
};