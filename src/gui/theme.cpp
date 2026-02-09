//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <gui/theme.hpp>
#include <data/settings.hpp>
#include <wx/appearance.h>
#include <wx/settings.h>

// ----------------------------------------------------------------------------- : Theme palette

static bool system_is_dark() {
  #if wxCHECK_VERSION(3, 1, 0)
    return wxSystemAppearance::Get().IsDark();
  #else
    return false;
  #endif
}

static ThemePalette light_palette() {
  ThemePalette palette;
  palette.background = wxColour(250, 250, 250);
  palette.text       = wxColour(24, 24, 24);
  palette.accent     = wxColour(0, 120, 215);
  palette.warning    = wxColour(197, 125, 0);
  palette.error      = wxColour(196, 43, 28);
  return palette;
}

static ThemePalette dark_palette() {
  ThemePalette palette;
  palette.background = wxColour(32, 32, 34);
  palette.text       = wxColour(230, 230, 232);
  palette.accent     = wxColour(86, 156, 214);
  palette.warning    = wxColour(255, 191, 0);
  palette.error      = wxColour(255, 99, 71);
  return palette;
}

static ThemePalette system_palette(bool dark_mode) {
  ThemePalette palette;
  palette.background = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
  palette.text       = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
  palette.accent     = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
  palette.warning    = dark_mode ? wxColour(255, 191, 0) : wxColour(197, 125, 0);
  palette.error      = dark_mode ? wxColour(255, 99, 71) : wxColour(196, 43, 28);
  return palette;
}

static bool color_is_dark(wxColour const& color) {
  int luminance = (color.Red() * 2126 + color.Green() * 7152 + color.Blue() * 722) / 10000;
  return luminance < 128;
}

static wxColour readable_text_for(wxColour const& background) {
  return color_is_dark(background) ? wxColour(255, 255, 255) : wxColour(0, 0, 0);
}

ThemePalette theme_palette() {
  bool system_dark = system_is_dark();
  switch (settings.theme_preference) {
    case THEME_LIGHT:
      return system_dark ? light_palette() : system_palette(false);
    case THEME_DARK:
      return system_dark ? system_palette(true) : dark_palette();
    case THEME_SYSTEM:
    default:
      return system_palette(system_dark);
  }
}

wxColour theme_color(ThemeColor color) {
  ThemePalette palette = theme_palette();
  switch (color) {
    case THEME_COLOR_TEXT:
      return palette.text;
    case THEME_COLOR_ACCENT:
      return palette.accent;
    case THEME_COLOR_WARNING:
      return palette.warning;
    case THEME_COLOR_ERROR:
      return palette.error;
    case THEME_COLOR_BACKGROUND:
    default:
      return palette.background;
  }
}

wxColour theme_selection_background() {
  if (settings.theme_preference == THEME_SYSTEM) {
    return wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
  }
  return theme_palette().accent;
}

wxColour theme_selection_text() {
  if (settings.theme_preference == THEME_SYSTEM) {
    return wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
  }
  return readable_text_for(theme_selection_background());
}

bool theme_is_dark() {
  if (settings.theme_preference == THEME_LIGHT) {
    return false;
  }
  if (settings.theme_preference == THEME_DARK) {
    return true;
  }
  return system_is_dark();
}

static void apply_theme_recursive(wxWindow* window, ThemePalette const& palette) {
  if (!window) return;
  window->SetBackgroundColour(palette.background);
  window->SetForegroundColour(palette.text);
  wxWindowList& children = window->GetChildren();
  for (wxWindowList::compatibility_iterator node = children.GetFirst(); node; node = node->GetNext()) {
    apply_theme_recursive(node->GetData(), palette);
  }
}

static void clear_theme_recursive(wxWindow* window) {
  if (!window) return;
  window->SetBackgroundColour(wxNullColour);
  window->SetForegroundColour(wxNullColour);
  wxWindowList& children = window->GetChildren();
  for (wxWindowList::compatibility_iterator node = children.GetFirst(); node; node = node->GetNext()) {
    clear_theme_recursive(node->GetData());
  }
}

void apply_theme_to_window(wxWindow* window) {
  if (!window) return;
  if (settings.theme_preference == THEME_SYSTEM) {
    clear_theme_recursive(window);
  } else {
    apply_theme_recursive(window, theme_palette());
  }
  window->Refresh();
}

void apply_theme_to_all_windows() {
  for (wxWindowList::compatibility_iterator node = wxTopLevelWindows.GetFirst(); node; node = node->GetNext()) {
    wxWindow* window = node->GetData();
    apply_theme_to_window(window);
  }
}
