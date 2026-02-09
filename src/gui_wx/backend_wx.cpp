//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#include <gui/backend.hpp>

#include "app_wx.hpp"

namespace gui {

class WxBackend final : public Backend {
public:
  int run(int argc, char** argv) override {
    return run_wx_app(argc, argv);
  }
};

std::unique_ptr<Backend> createBackend() {
  return std::make_unique<WxBackend>();
}

} // namespace gui
