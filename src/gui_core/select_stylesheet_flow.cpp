#include <gui_core/select_stylesheet_flow.hpp>

#include <data/game.hpp>
#include <data/settings.hpp>
#include <data/stylesheet.hpp>
#include <util/io/package_manager.hpp>

namespace gui_core {
namespace {
StyleSheetP open_stylesheet_or_null(const String& full_name) {
  if (full_name.empty()) {
    return StyleSheetP();
  }
  try {
    return package_manager.open<StyleSheet>(full_name);
  } catch (const Error&) {
    return StyleSheetP();
  }
}

StyleSheetP select_by_policy(const Game& game) {
  const String& preferred = settings.gameSettingsFor(game).default_stylesheet;
  if (!preferred.empty()) {
    StyleSheetP preferred_sheet =
      open_stylesheet_or_null(game.name() + _("-") + preferred + _(".mse-style"));
    if (preferred_sheet) {
      return preferred_sheet;
    }
  }

  vector<PackagedP> matches;
  package_manager.findMatching(game.name() + _("-*.mse-style"), matches);
  for (const PackagedP& candidate : matches) {
    StyleSheetP sheet = open_stylesheet_or_null(candidate->relativeFilename());
    if (sheet) {
      return sheet;
    }
  }

  return StyleSheetP();
}
} // namespace

StyleSheetP select_stylesheet(const Game& game, const String& failed_name, MissingStyleSheetPresenter* presenter) {
  StyleSheetP selected = select_by_policy(game);
  if (selected) {
    return selected;
  }
  if (!presenter) {
    return StyleSheetP();
  }
  return presenter->selectMissingStyleSheet(game, failed_name);
}

} // namespace gui_core
