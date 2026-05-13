//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make card games                |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#include <gui/backend.hpp>

#include <cstdlib>

int main(int argc, char** argv) {
  auto backend = gui::createBackend();
  if (!backend) {
    return EXIT_FAILURE;
  }
  return backend->run(argc, argv);
}
