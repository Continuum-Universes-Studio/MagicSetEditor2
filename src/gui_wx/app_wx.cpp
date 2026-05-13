//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make card games                |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/io/package_manager.hpp>
#include <util/spell_checker.hpp>
#include <data/game.hpp>
#include <data/set.hpp>
#include <data/settings.hpp>
#include <data/locale.hpp>
#include <data/installer.hpp>
#include <data/format/formats.hpp>
#include <data/font.hpp>
#include <cli/cli_main.hpp>
#include <cli/text_io_handler.hpp>
#include <gui/welcome_window.hpp>
#include <gui/downloadable_installers.hpp>
#include <gui/packages_window.hpp>
#include <gui/set/window.hpp>
#include <gui/symbol/window.hpp>
#include <gui/thumbnail_thread.hpp>
#include <gui/theme.hpp>
#include <gui_core/startup_flow.hpp>
#include <gui_wx/select_stylesheet_wx.hpp>
#include <wx/fs_inet.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/socket.h>

#include "app_wx.hpp"

ScriptValueP export_set(SetP const& set, vector<CardP> const& cards, ExportTemplateP const& exp, String const& outname);

// ----------------------------------------------------------------------------- : Main function/class

/// The application class for MSE.
/** This class is used by wxWidgets as a kind of 'main function'
 */
class MSEWxApp : public wxApp {
public:
  /// Do nothing. The command line parsing, etc. is done in OnRun
  bool OnInit() override { return true; }
  /// Main startup function of the program
  /** Use OnRun instead of OnInit, so we can determine whether or not we need a main loop
   *  Also, OnExit is always run.
   */
  int OnRun() override;
  /// Actually start the GUI mainloop
  int runGUI();
  /// On exit: write the settings to the config file
  int OnExit() override;
  /// On exception: display error message
  void HandleEvent(wxEvtHandler *handler, wxEventFunction func, wxEvent& event) const override;
  /// Hack around some wxWidget idiocies
  int FilterEvent(wxEvent& ev) override;
  /// Apply theme to newly created windows
  void onWindowCreate(wxWindowCreateEvent& event);
  /// Fancier assert
  #if defined(_MSC_VER) && defined(_DEBUG) && defined(_CRT_WIDE)
    void OnAssert(const wxChar *file, int line, const wxChar *cond, const wxChar *msg) override;
  #endif
};

IMPLEMENT_APP_NO_MAIN(MSEWxApp)

// ----------------------------------------------------------------------------- : Checks

void nag_about_ascii_version() {
  #if !defined(UNICODE) && defined(__WXMSW__)
    // windows 2000/XP/Vista/... users shouldn't use the 9x version
    OSVERSIONINFO info;
    info.dwOSVersionInfoSize = sizeof(info);
    GetVersionEx(&info);
    if (info.dwMajorVersion >= 5) {
      queue_message(MESSAGE_WARNING,
        _("This build of Magic Set Editor is intended for Windows 95/98/ME systems.\n")
        _("It is recommended that you download the appropriate MSE version for your Windows version."));
    }
  #endif
}

// ----------------------------------------------------------------------------- : Initialization

int MSEWxApp::OnRun() {
  try {
    #ifdef __WXMSW__
      SetAppName(_("Magic Set Editor"));
    #else
      SetAppName(_("magicseteditor"));
    #endif
    FontRef::PreloadResourceFonts(true);
    wxInitAllImageHandlers();
    wxFileSystem::AddHandler(new wxInternetFSHandler);
    wxSocketBase::Initialize();
    init_script_variables();
    init_file_formats();
    cli.init(argc, argv);
    package_manager.init();
    settings.read();
    SetAppearance((Appearance)settings.dark_mode_type);
    the_locale = Locale::byName(settings.locale);
    Bind(wxEVT_CREATE, &MSEWxApp::onWindowCreate, this);
    gui_wx::install_missing_stylesheet_presenter();
    nag_about_ascii_version();

    class WxStartupPresenter final : public gui_core::StartupPresenter {
    public:
      explicit WxStartupPresenter(MSEWxApp& app) : app_(app) {}

      int showWelcomeWindow() override {
        (new WelcomeWindow())->Show();
        return app_.runGUI();
      }

      int showSymbolEditor(const String& file) override {
        Window* wnd = file.empty() ? new SymbolWindow(nullptr) : new SymbolWindow(nullptr, file);
        wnd->Show();
        return app_.runGUI();
      }

      int showSetEditor(const String& file) override {
        Window* wnd = new SetWindow(nullptr, import_set(file));
        wnd->Show();
        return app_.runGUI();
      }

      int showInstaller(const String& installer_file, InstallType install_type) override {
        (void)install_type;
        InstallerP installer = open_package<Installer>(installer_file);
        PackagesWindow wnd(nullptr, installer);
        wnd.ShowModal();
        return EXIT_SUCCESS;
      }

    private:
      MSEWxApp& app_;
    } presenter(*this);

    const gui_core::StartupRequest request =
      gui_core::parse_startup_request(argc, argv, settings.install_type);
    const wxScopedCharBuffer executable_name = argc > 0 ? argv[0].utf8_str() : wxScopedCharBuffer();
    return gui_core::run_startup_request(
      request,
      presenter,
      executable_name ? executable_name.data() : "magicseteditor"
    );

  } CATCH_ALL_ERRORS(true);
  cli.print_pending_errors();
  return EXIT_FAILURE;
}

int MSEWxApp::runGUI() {
  return wxApp::OnRun();
}

// ----------------------------------------------------------------------------- : Exit

int MSEWxApp::OnExit() {
  thumbnail_thread.abortAll();
  settings.write();
  package_manager.destroy();
  SpellChecker::destroyAll();
  return 0;
}

// ----------------------------------------------------------------------------- : Exception handling

void MSEWxApp::HandleEvent(wxEvtHandler *handler, wxEventFunction func, wxEvent& event) const {
  try {
    wxApp::HandleEvent(handler, func, event);
  } CATCH_ALL_ERRORS(true);
}

#if defined(_MSC_VER) && defined(_DEBUG) && defined(_CRT_WIDE)
  // Print assert failures to debug output
  void MSEWxApp::OnAssert(const wxChar *file, int line, const wxChar *cond, const wxChar *msg) {
    #ifdef UNICODE
      msvc_assert(msg, cond, file, line);
    #else
      wchar_t file_[1024]; mbstowcs(file_,file,1023);
      wchar_t cond_[1024]; mbstowcs(cond_,cond,1023);
      wchar_t msg_ [1024]; mbstowcs(msg_, msg, 1023);
      msvc_assert(msg_, cond_, file_, line);
    #endif
  }
#endif

// ----------------------------------------------------------------------------- : Events

int MSEWxApp::FilterEvent(wxEvent& ev) {
  /*if (ev.GetEventType() == wxEVT_MOUSE_CAPTURE_LOST) {
    return 1;
  } else {
    return -1;
  }*/
  return -1;
}

void MSEWxApp::onWindowCreate(wxWindowCreateEvent& event) {
  apply_theme_to_window(event.GetWindow());
  event.Skip();
}

int run_wx_app(int argc, char** argv) {
  return wxEntry(argc, argv);
}
