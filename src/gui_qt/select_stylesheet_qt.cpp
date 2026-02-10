//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#include "select_stylesheet_qt.hpp"

#include <gui/select_stylesheet.hpp>
#include <gui_core/select_stylesheet_flow.hpp>

namespace gui_qt {
namespace {
class QtMissingStyleSheetPresenter final : public gui_core::MissingStyleSheetPresenter {
public:
  StyleSheetP selectMissingStyleSheet(const Game& game, const String& failed_name) override {
    (void)game;
    (void)failed_name;
    return StyleSheetP();
  }
};
} // namespace

void install_missing_stylesheet_presenter() {
  gui::set_missing_stylesheet_presenter(std::make_unique<QtMissingStyleSheetPresenter>());
}

} // namespace gui_qt
