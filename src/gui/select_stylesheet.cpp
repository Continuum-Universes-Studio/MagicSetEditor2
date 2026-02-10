#include <gui/select_stylesheet.hpp>

#include <gui_core/select_stylesheet_flow.hpp>
#include <data/stylesheet.hpp>

namespace gui {
namespace {
std::unique_ptr<gui_core::MissingStyleSheetPresenter>& presenter_slot() {
  static std::unique_ptr<gui_core::MissingStyleSheetPresenter> presenter;
  return presenter;
}
} // namespace

StyleSheetP select_stylesheet(const Game& game, const String& failed_name) {
  return gui_core::select_stylesheet(game, failed_name, presenter_slot().get());
}

void set_missing_stylesheet_presenter(std::unique_ptr<gui_core::MissingStyleSheetPresenter> presenter) {
  presenter_slot() = std::move(presenter);
}

} // namespace gui
