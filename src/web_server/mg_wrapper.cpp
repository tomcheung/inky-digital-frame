#include "mg_wrapper.hpp"
#include "mongoose.h"

MgWrapper::MgWrapper(): mgr(new mg_mgr) {
  mg_mgr_init(mgr);
}