#include "select_stylesheet_wx.hpp"

#include <data/stylesheet.hpp>
#include <gui/new_window.hpp>
#include <gui/select_stylesheet.hpp>
#include <gui_core/select_stylesheet_flow.hpp>

namespace gui_wx {
namespace {
class WxMissingStyleSheetPresenter final : public gui_core::MissingStyleSheetPresenter {
public:
  StyleSheetP selectMissingStyleSheet(const Game& game, const String& failed_name) override {
    SelectStyleSheetWindow wnd(nullptr, game, failed_name);
    wnd.ShowModal();
    return wnd.stylesheet;
  }
};
} // namespace

void install_missing_stylesheet_presenter() {
  gui::set_missing_stylesheet_presenter(std::make_unique<WxMissingStyleSheetPresenter>());
}

} // namespace gui_wx
