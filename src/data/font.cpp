//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make card games                |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/font.hpp>
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/font.h>

// ----------------------------------------------------------------------------- : Font

FontRef::FontRef()
  : name()
  , size(1)
  , underline(false)
  , strikethrough(false)
  , scale_down_to(100000)
  , max_stretch(1.0)
  , color(Color(0,0,0))
  , shadow_color(Color(0,0,0))
  , shadow_displacement_x(0)
  , shadow_displacement_y(0)
  , shadow_blur(0)
  , stroke_color(Color(0,0,0))
  , stroke_radius(0)
  , stroke_blur(0)
  , separator_color(Color(0,0,0,128))
  , flags(FONT_NORMAL)
{}

bool FontRef::PreloadResourceFonts(bool recursive) {
#if wxUSE_PRIVATE_FONTS
  String pathSeparator(wxFileName::GetPathSeparator());
  String appPath(wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath());
  wxDir appDir(appPath);
  if (!appDir.IsOpened()) return true;

  bool preloadHadErrors = false;
  wxString folder;
  bool cont = appDir.GetFirst(&folder, _(""), wxDIR_DIRS);
  while (cont)
  {
    if (folder.Lower().Contains("fonts")) {
      String folderPath = appPath + pathSeparator + folder + pathSeparator;
      
      // tally fonts
      vector<String> fontFilePaths;
      TallyResourceFonts(folderPath, fontFilePaths, recursive);

      // load fonts
      for (const String& fontFilePath : fontFilePaths) {
        if (!wxFont::AddPrivateFont(fontFilePath)) {
          preloadHadErrors = true;
        }
      }
    }
    cont = appDir.GetNext(&folder);
  }

  return preloadHadErrors;

#endif // wxUSE_PRIVATE_FONTS
  return false;
}

void FontRef::TallyResourceFonts(String fontsDirectoryPath, vector<String>& fontFilePaths, bool recursive) {
  wxDir fontsDirectory(fontsDirectoryPath);
  String fontFileName = _("");
  bool hasNext = fontsDirectory.GetFirst(&fontFileName);
  while (hasNext) {
    String fontFilePath = fontsDirectoryPath + fontFileName;
    if (wxDirExists(fontFilePath)) {
      if (recursive) {
        TallyResourceFonts(fontFilePath + wxFileName::GetPathSeparator(), fontFilePaths, true);
      }
    }
    else if (fontFilePath.EndsWith(_(".ttf")) || fontFilePath.EndsWith(_(".otf"))) {
      fontFilePaths.push_back(fontFilePath);
    }
    hasNext = fontsDirectory.GetNext(&fontFileName);
  }
}

bool FontRef::update(Context& ctx) {
  bool changes = false;
  changes |= name                 .update(ctx);
  changes |= italic_name          .update(ctx);
  changes |= size                 .update(ctx);
  changes |= weight               .update(ctx);
  changes |= style                .update(ctx);
  changes |= underline            .update(ctx);
  changes |= strikethrough        .update(ctx);
  changes |= color                .update(ctx);
  changes |= shadow_color         .update(ctx);
  changes |= shadow_displacement_x.update(ctx);
  changes |= shadow_displacement_y.update(ctx);
  changes |= shadow_blur          .update(ctx);
  changes |= stroke_color         .update(ctx);
  changes |= stroke_radius        .update(ctx);
  changes |= stroke_blur          .update(ctx);
  flags = (flags & ~FONT_BOLD & ~FONT_ITALIC)
        | (weight() == _("bold")   ? FONT_BOLD   : FONT_NORMAL)
        | (style()  == _("italic") ? FONT_ITALIC : FONT_NORMAL);
  return changes;
}
void FontRef::initDependencies(Context& ctx, const Dependency& dep) const {
  name                 .initDependencies(ctx, dep);
  italic_name          .initDependencies(ctx, dep);
  size                 .initDependencies(ctx, dep);
  weight               .initDependencies(ctx, dep);
  style                .initDependencies(ctx, dep);
  underline            .initDependencies(ctx, dep);
  strikethrough        .initDependencies(ctx, dep);
  color                .initDependencies(ctx, dep);
  shadow_color         .initDependencies(ctx, dep);
  shadow_displacement_x.initDependencies(ctx, dep);
  shadow_displacement_y.initDependencies(ctx, dep);
  shadow_blur          .initDependencies(ctx, dep);
  stroke_color         .initDependencies(ctx, dep);
  stroke_blur          .initDependencies(ctx, dep);
  stroke_radius        .initDependencies(ctx, dep);
}

FontRefP FontRef::make(int add_flags, bool add_underline, bool add_strikethrough, String const* other_family, Color const* other_color, double const* other_size) const {
  FontRefP f(new FontRef(*this));
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
    f->shadow_displacement_x = 0; // no shadow
    f->shadow_displacement_y = 0; // no shadow
  }
  if (add_underline) {
    f->underline = true;
  }
  if (add_strikethrough) {
    f->strikethrough = true;
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

wxFont FontRef::toWxFont(double scale) const {
  double point_size = scale * size;
  int size_i = to_int(scale * size);
  wxFontWeight weight_i = flags & FONT_BOLD   ? wxFONTWEIGHT_BOLD  : wxFONTWEIGHT_NORMAL;
  wxFontStyle style_i  = flags & FONT_ITALIC ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL;
  // make font
  wxFont font;

  if (flags & FONT_CODE) {
    if (size_i < 2) {
      font = wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, weight_i, underline(), _("Courier New"));
      if (strikethrough()) font.MakeStrikethrough();
      return font;
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
  if (strikethrough()) font.MakeStrikethrough();
  return font;
}

IMPLEMENT_REFLECTION_NO_SCRIPT(FontRef) {
  REFLECT(name);
  REFLECT(size);
  REFLECT(weight);
  REFLECT(style);
  REFLECT(underline);
  REFLECT(strikethrough);
  REFLECT(italic_name);
  REFLECT(color);
  REFLECT(scale_down_to);
  REFLECT(max_stretch);
  REFLECT(shadow_color);
  REFLECT(shadow_displacement_x);
  REFLECT(shadow_displacement_y);
  REFLECT(shadow_blur);
  REFLECT(stroke_color);
  REFLECT(stroke_radius);
  REFLECT(stroke_blur);
  REFLECT(separator_color);
}
