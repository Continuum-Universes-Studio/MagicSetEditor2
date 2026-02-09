//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#include <util/prec.hpp>
#include <gui/select_stylesheet.hpp>

#include <data/game.hpp>
#include <data/settings.hpp>
#include <data/stylesheet.hpp>
#include <util/io/package_manager.hpp>

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
} // namespace

StyleSheetP select_stylesheet(const Game& game, const String& failed_name) {
  (void)failed_name;
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
