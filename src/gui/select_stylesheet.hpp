//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

#include <memory>

#include <util/prec.hpp>

class Game;
DECLARE_POINTER_TYPE(StyleSheet);

namespace gui_core {
class MissingStyleSheetPresenter;
}

namespace gui {

/// Select an alternative stylesheet when the requested one is missing.
StyleSheetP select_stylesheet(const Game& game, const String& failed_name);

/// Configure backend-specific UI presenter for missing stylesheet selection.
void set_missing_stylesheet_presenter(std::unique_ptr<gui_core::MissingStyleSheetPresenter> presenter);

} // namespace gui
