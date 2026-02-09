//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/font.hpp>

// ----------------------------------------------------------------------------- : Font

Font::Font()
  : name()
  , size(1)
  , underline(false)
  , scale_down_to(100000)
  , max_stretch(1.0)
  , color(Color(0,0,0))
  , shadow_displacement(0,0)
  , shadow_blur(0)
  , separator_color(Color(0,0,0,128))
  , flags(FONT_NORMAL)
{}

bool Font::update(Context& ctx) {
  bool changes = false;
  changes |= name        .update(ctx);
  changes |= italic_name .update(ctx);
  changes |= size        .update(ctx);
  changes |= weight      .update(ctx);
  changes |= style       .update(ctx);
  changes |= underline   .update(ctx);
  changes |= color       .update(ctx);
  changes |= shadow_color.update(ctx);
  flags = (flags & ~FONT_BOLD & ~FONT_ITALIC)
        | (weight() == _("bold")   ? FONT_BOLD   : FONT_NORMAL)
        | (style()  == _("italic") ? FONT_ITALIC : FONT_NORMAL);
  return changes;
}
void Font::initDependencies(Context& ctx, const Dependency& dep) const {
  name        .initDependencies(ctx, dep);
  italic_name .initDependencies(ctx, dep);
  size        .initDependencies(ctx, dep);
  weight      .initDependencies(ctx, dep);
  style       .initDependencies(ctx, dep);
  underline   .initDependencies(ctx, dep);
  color       .initDependencies(ctx, dep);
  shadow_color.initDependencies(ctx, dep);
}

FontP Font::make(int add_flags, bool add_underline, String const* other_family, Color const* other_color, double const* other_size) const {
  FontP f(new Font(*this));
  f->flags |= add_flags;
  if (add_flags & FONT_CODE_STRING) {
    f->color = Color(0,0,100);
  }
  if (add_flags & FONT_CODE) {
    f->color = Color(128,0,0);
  }
  if (add_flags & FONT_CODE_KW) {
    f->color = Color(158,100,0);
    f->flags |= FONT_BOLD;
  }
  if (add_flags & FONT_SOFT) {
    f->color = f->separator_color;
    f->shadow_displacement = RealSize(0,0); // no shadow
  }
  if (add_underline) {
    f->underline = true;
  }
  if (other_color) {
    f->color = *other_color;
  }
  if (other_size) {
    f->size = *other_size;
  }
  if (other_family && !other_family->empty()) {
    f->name = *other_family;
  }
  return f;
}

static const String BOLD_STRING         = _(" Bold");
static const String ITALIC_STRING       = _(" Italic");
static const String OBLIQUE_STRING      = _(" Oblique");
static const String BOLD_ITALIC_STRING  = _(" Bold Italic");
static const String BOLD_OBLIQUE_STRING = _(" Bold Oblique");
static const String DASH_ITALIC_STRING  = _("-Italic");
static const String DASH_OBLIQUE_STRING = _("-Oblique");

static bool removeSuffix(String& text, const String& suffix) {
  if (!text.EndsWith(suffix)) return false;
  text = text.Left(text.length() - suffix.length());
  return true;
}

static void normalizeFontFace(String& familyName, wxFontWeight& weight, wxFontStyle& style) {
  #ifdef __WXGTK__
    if (removeSuffix(familyName, BOLD_ITALIC_STRING) || removeSuffix(familyName, BOLD_OBLIQUE_STRING)) {
      weight = wxFONTWEIGHT_BOLD;
      style = wxFONTSTYLE_ITALIC;
      return;
    }
  #endif
  if (removeSuffix(familyName, BOLD_STRING)) {
    weight = wxFONTWEIGHT_BOLD;
  }
  #ifdef __WXGTK__
    if (removeSuffix(familyName, ITALIC_STRING)
     || removeSuffix(familyName, OBLIQUE_STRING)
     || removeSuffix(familyName, DASH_ITALIC_STRING)
     || removeSuffix(familyName, DASH_OBLIQUE_STRING)) {
      style = wxFONTSTYLE_ITALIC;
    }
  #endif
}

wxFont Font::toWxFont(double scale) const {
  double point_size = scale * size;
  int size_i = to_int(scale * size);
  wxFontWeight weight_i = flags & FONT_BOLD   ? wxFONTWEIGHT_BOLD  : wxFONTWEIGHT_NORMAL;
  wxFontStyle style_i  = flags & FONT_ITALIC ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL;
  // make font
  wxFont font;

  if (flags & FONT_CODE) {
    if (size_i < 2) {
      return wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, weight_i, underline(), _("Courier New"));
    } else {
      font = wxFont(size_i, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, weight_i, underline(), _("Courier New"));
    }
  } else if (name().empty()) {
    font = *wxNORMAL_FONT;
    point_size = size > 1 ? point_size : scale * font.GetPointSize();
    font.SetPointSize(to_int(point_size));
  } else if (flags & FONT_ITALIC && !italic_name().empty()) {
    #ifdef __WXGTK__
      String familyName = italic_name();
      wxFontStyle italic_style_i = wxFONTSTYLE_ITALIC;
      normalizeFontFace(familyName, weight_i, italic_style_i);
      font = wxFont(size_i, wxFONTFAMILY_DEFAULT, italic_style_i, weight_i, underline(), familyName);
    #else
      font = wxFont(size_i, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, weight_i, underline(), italic_name());
    #endif
  } else {
    String familyName = name();
    normalizeFontFace(familyName, weight_i, style_i);
    font = wxFont(size_i, wxFONTFAMILY_DEFAULT, style_i, weight_i, underline(), familyName);
  }
  // fix size
  #if defined(__WXMSW__) || defined(__WXGTK__)
    // Card/template fonts are sized in 96-DPI pixels. wxGTK point fonts follow
    // desktop DPI scaling, which makes preview text too large on HiDPI displays.
    int pixel_height = (int)(point_size * 96.0 / 72.0 + 0.5);
    if (pixel_height < 1) pixel_height = 1;
    font.SetPixelSize(wxSize(0, pixel_height));
  #endif
  return font;
}

IMPLEMENT_REFLECTION_NO_SCRIPT(Font) {
  REFLECT(name);
  REFLECT(size);
  REFLECT(weight);
  REFLECT(style);
  REFLECT(underline);
  REFLECT(italic_name);
  REFLECT(color);
  REFLECT(scale_down_to);
  REFLECT(max_stretch);
  REFLECT_N("shadow_displacement_x", shadow_displacement.width);
  REFLECT_N("shadow_displacement_y", shadow_displacement.height);
  REFLECT(shadow_color);
  REFLECT(shadow_blur);
  REFLECT(separator_color);
}
