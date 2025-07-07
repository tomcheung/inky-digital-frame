#pragma once

struct mg_mgr;

class MgWrapper {
  public:
    MgWrapper();
    mg_mgr* mgr;
};