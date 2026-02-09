//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

#include <memory>

namespace gui {

class Backend {
public:
  virtual ~Backend() = default;
  virtual int run(int argc, char** argv) = 0;
};

std::unique_ptr<Backend> createBackend();

} // namespace gui
