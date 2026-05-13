//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

// ----------------------------------------------------------------------------- : Includes

#include <wx/colour.h>
#include <wx/window.h>

// ----------------------------------------------------------------------------- : Theme palette

struct ThemePalette {
  wxColour background;
  wxColour text;
  wxColour accent;
  wxColour warning;
  wxColour error;
};

enum ThemeColor {
  THEME_COLOR_BACKGROUND,
  THEME_COLOR_TEXT,
  THEME_COLOR_ACCENT,
  THEME_COLOR_WARNING,
  THEME_COLOR_ERROR
};

ThemePalette theme_palette();
wxColour theme_color(ThemeColor color);
wxColour theme_selection_background();
wxColour theme_selection_text();
bool theme_is_dark();

/// Apply the current theme to a window and its children.
void apply_theme_to_window(wxWindow* window);
/// Apply the current theme to all top-level windows.
void apply_theme_to_all_windows();
