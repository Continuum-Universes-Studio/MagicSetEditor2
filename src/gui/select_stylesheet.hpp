//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

#include <util/prec.hpp>

class Game;
DECLARE_POINTER_TYPE(StyleSheet);

/// Select an alternative stylesheet when the requested one is missing.
StyleSheetP select_stylesheet(const Game& game, const String& failed_name);
