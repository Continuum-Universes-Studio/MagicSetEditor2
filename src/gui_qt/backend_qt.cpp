//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#include <gui/backend.hpp>

#include <cli/cli_main.hpp>
#include <cli/text_io_handler.hpp>
#include <data/format/formats.hpp>
#include <data/installer.hpp>
#include <data/locale.hpp>
#include <data/set.hpp>
#include <data/settings.hpp>
#include <gui_qt/windows.hpp>
#include <gui_wx/app_wx.hpp>
#include <util/prec.hpp>
#include <util/io/package_manager.hpp>
#include <util/spell_checker.hpp>
#include <wx/filename.h>
#include <wx/filefn.h>
#include <wx/init.h>

#include <QApplication>
#include <QDateTime>
#include <QMessageLogContext>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>

ScriptValueP export_set(SetP const& set, vector<CardP> const& cards, ExportTemplateP const& exp, String const& outname);

namespace gui {

namespace {
std::string resolve_log_dir() {
  const char* home = std::getenv("HOME");
#ifdef _WIN32
  if (!home) {
    home = std::getenv("USERPROFILE");
  }
  if (!home) {
    home = std::getenv("APPDATA");
  }
#endif
  if (!home || std::string(home).empty()) {
    return ".";
  }
  return std::string(home) + "/.magicseteditor";
}

std::ofstream& boot_log() {
  static std::ofstream log_stream;
  static bool initialized = false;
  if (!initialized) {
    initialized = true;
    try {
      const std::string log_dir = resolve_log_dir();
      std::filesystem::create_directories(log_dir);
      const std::string log_path = log_dir + "/boot.log";
      log_stream.open(log_path, std::ios::out | std::ios::app);
      if (log_stream.is_open()) {
        log_stream << "\n--- Magic Set Editor boot log (Qt6 backend) ---\n";
      }
    } catch (...) {
      // Best-effort logging only.
    }
  }
  return log_stream;
}

void log_line(const std::string& message) {
  auto& stream = boot_log();
  if (!stream.is_open()) {
    return;
  }
  const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  stream << std::string(std::ctime(&now)).substr(0, 24) << " | " << message << "\n";
}

const char* qt_log_level(QtMsgType type) {
  switch (type) {
    case QtDebugMsg:
      return "DEBUG";
    case QtInfoMsg:
      return "INFO";
    case QtWarningMsg:
      return "WARN";
    case QtCriticalMsg:
      return "CRITICAL";
    case QtFatalMsg:
      return "FATAL";
    default:
      return "UNKNOWN";
  }
}

void qt_message_handler(QtMsgType type, const QMessageLogContext& context, const QString& message) {
  std::string output = std::string("Qt ") + qt_log_level(type) + ": " + message.toStdString();
  if (context.file && context.line > 0) {
    output += " (" + std::string(context.file) + ":" + std::to_string(context.line) + ")";
  }
  log_line(output);
}

int run_gui(QApplication& app) {
  return app.exec();
}

void show_and_run(QApplication& app, QWidget* window) {
  window->show();
  run_gui(app);
}

QString toQString(const String& value) {
  if (value.empty()) {
    return QString();
  }
  const wxScopedCharBuffer utf8 = value.ToUTF8();
  return QString::fromUtf8(utf8.data(), static_cast<qsizetype>(utf8.length()));
}

struct ScopedCleanup {
  ~ScopedCleanup() {
    settings.write();
    package_manager.destroy();
    SpellChecker::destroyAll();
  }
};
} // namespace

class QtBackend final : public Backend {
public:
  int run(int argc, char** argv) override {
    log_line("Starting Qt backend.");
    if (argc <= 1 || (argc == 2 && std::string(argv[1]) == "--color")) {
      log_line("No startup arguments provided; handing off to wxWidgets welcome window.");
      return run_wx_app(argc, argv);
    }
    qInstallMessageHandler(qt_message_handler);
    QApplication app(argc, argv);
    log_line("QApplication created.");
    wxInitializer wx_init;
    if (!wx_init) {
      log_line("Failed to initialize wxWidgets.");
      return EXIT_FAILURE;
    }

    init_script_variables();
    init_file_formats();
    cli.init();
    package_manager.init();
    settings.read();
    the_locale = Locale::byName(settings.locale);
    ScopedCleanup cleanup;

    try {
      // interpret command line
      log_line("Parsing command line arguments.");
      vector<String> args;
      for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
        if (args.back() == _("--color")) args.pop_back();
      }
      if (!args.empty()) {
        const String& arg = args[0];
        wxFileName f(arg.Mid(0, arg.find_last_not_of(_("\\/")) + 1));
        if (f.GetExt() == _("mse-symbol")) {
          auto* wnd = new gui_qt::SymbolWindow(toQString(arg));
          show_and_run(app, wnd);
          return EXIT_SUCCESS;
        } else if (f.GetExt() == _("mse-set") || f.GetExt() == _("mse") || f.GetExt() == _("set")) {
          auto* wnd = new gui_qt::SetWindow(toQString(arg));
          show_and_run(app, wnd);
          return EXIT_SUCCESS;
        } else if (f.GetExt() == _("mse-installer")) {
          gui_qt::PackagesWindow wnd(toQString(arg));
          wnd.exec();
          return EXIT_SUCCESS;
        } else if (f.GetExt() == _("mse-script")) {
          if (!run_script_file(arg)) return EXIT_FAILURE;
          if (cli.shown_errors()) return EXIT_FAILURE;
          return EXIT_SUCCESS;
        } else if (arg == _("--symbol-editor")) {
          auto* wnd = new gui_qt::SymbolWindow();
          show_and_run(app, wnd);
          return EXIT_SUCCESS;
        } else if (arg == _("--create-installer")) {
          Installer inst;
          FOR_EACH(arg, args) {
            if (!starts_with(arg, _("--"))) {
              inst.addPackage(arg);
            }
          }
          if (inst.prefered_filename.empty()) {
            throw Error(_("Specify packages to include in installer"));
          } else {
            inst.saveAs(inst.prefered_filename, false);
          }
          return EXIT_SUCCESS;
        } else if (arg == _("--help") || arg == _("-?")) {
          cli << _("Magic Set Editor\n\n");
          cli << _("Usage: ") << BRIGHT << argv[0] << NORMAL << _(" [") << PARAM << _("OPTIONS") << NORMAL << _("]");
          cli << _("\n\n  no options");
          cli << _("\n         \tStart the MSE user interface showing the welcome window.");
          cli << _("\n\n  ") << BRIGHT << _("-?") << NORMAL << _(", ")
                             << BRIGHT << _("--help") << NORMAL;
          cli << _("\n         \tShows this help screen.");
          cli << _("\n\n  ") << BRIGHT << _("-v") << NORMAL << _(", ")
                             << BRIGHT << _("--version") << NORMAL;
          cli << _("\n         \tShow version information.");
          cli << _("\n\n  ") << PARAM << _("FILE") << FILE_EXT << _(".mse-set") << NORMAL << _(", ")
                             << PARAM << _("FILE") << FILE_EXT << _(".set") << NORMAL << _(", ")
                             << PARAM << _("FILE") << FILE_EXT << _(".mse") << NORMAL;
          cli << _("\n         \tLoad the set file in the MSE user interface.");
          cli << _("\n\n  ") << PARAM << _("FILE") << FILE_EXT << _(".mse-symbol") << NORMAL;
          cli << _("\n         \tLoad the symbol into the MSE symbol editor.");
          cli << _("\n\n  ") << PARAM << _("FILE") << FILE_EXT << _(".mse-installer")
                             << NORMAL << _(" [") << BRIGHT << _("--local") << NORMAL << _("]");
          cli << _("\n         \tInstall the packages from the installer.");
          cli << _("\n         \tIf the ") << BRIGHT << _("--local") << NORMAL << _(" flag is passed, install packages for this user only.");
          cli << _("\n\n  ") << PARAM << _("FILE") << FILE_EXT << _(".mse-script") << NORMAL;
          cli << _("\n         \tRun a script file.");
          cli << _("\n\n  ") << BRIGHT << _("--symbol-editor") << NORMAL;
          cli << _("\n         \tShow the symbol editor instead of the welcome window.");
          cli << _("\n\n  ") << BRIGHT << _("--create-installer") << NORMAL << _(" [")
                             << PARAM << _("OUTFILE") << FILE_EXT << _(".mse-installer") << NORMAL << _("] [")
                             << PARAM << _("PACKAGE") << NORMAL << _(" [") << PARAM << _("PACKAGE") << NORMAL << _(" ...]]");
          cli << _("\n         \tCreate an instaler, containing the listed packages.");
          cli << _("\n         \tIf no output filename is specified, the name of the first package is used.");
          cli << _("\n\n  ") << BRIGHT << _("--export") << NORMAL << PARAM << _(" TEMPLATE SETFILE ") << NORMAL << _(" [") << PARAM << _("OUTFILE") << NORMAL << _("]");
          cli << _("\n         \tExport a set using an export template.");
          cli << _("\n         \tIf no output filename is specified, the result is written to stdout.");
          cli << _("\n\n  ") << BRIGHT << _("--export-images") << NORMAL << PARAM << _(" FILE") << NORMAL << _(" [") << PARAM << _("IMAGE") << NORMAL << _("]");
          cli << _("\n         \tExport the cards in a set to image files,");
          cli << _("\n         \tIMAGE is the same format as for 'export all card images'.");
          cli << _("\n\n  ") << BRIGHT << _("--cli") << NORMAL << _(" [")
                             << PARAM << _("FILE") << NORMAL << _("] [")
                             << BRIGHT << _("--quiet") << NORMAL << _("] [")
                             << BRIGHT << _("--raw") << NORMAL << _("]");
          cli << _("\n         \tStart the command line interface for performing commands on the set file.");
          cli << _("\n         \tUse ") << BRIGHT << _("-q") << NORMAL << _(" or ") << BRIGHT << _("--quiet") << NORMAL << _(" to supress the startup banner and prompts.");
          cli << _("\n         \tUse ") << BRIGHT << _("-raw") << NORMAL << _(" for raw output mode.");
          cli << _("\n\nRaw output mode is intended for use by other programs:");
          cli << _("\n    - The only output is only in response to commands.");
          cli << _("\n    - For each command a single 'record' is written to the standard output.");
          cli << _("\n    - The record consists of:");
          cli << _("\n        - A line with an integer status code, 0 for ok, 1 for warnings, 2 for errors");
          cli << _("\n        - A line containing an integer k, the number of lines to follow");
          cli << _("\n        - k lines, each containing UTF-8 encoded string data.");
          cli << ENDL;
          cli.flush();
          return EXIT_SUCCESS;
        } else if (arg == _("--version") || arg == _("-v") || arg == _("-V")) {
          cli << _("Magic Set Editor\n");
          cli << _("Version ") << app_version.toString() << version_suffix << ENDL;
          cli.flush();
          return EXIT_SUCCESS;
        } else if (arg == _("--cli")) {
          SetP set;
          bool quiet = false;
          for (size_t i = 1; i < args.size(); ++i) {
            String const& arg = args[i];
            wxFileName f(arg);
            if (f.GetExt() == _("mse-set") || f.GetExt() == _("mse") || f.GetExt() == _("set")) {
              set = import_set(arg);
            } else if (arg == _("-q") || arg == _("--quiet")) {
              quiet = true;
            } else if (arg == _("-r") || arg == _("--raw")) {
              quiet = true;
              cli.enableRaw();
            }
          }
          CLISetInterface cli_interface(set, quiet);
          return EXIT_SUCCESS;
        } else if (arg == _("--export-images")) {
          if (args.size() < 2) {
            handle_error(Error(_("No input file specified for --export")));
            return EXIT_FAILURE;
          }
          SetP set = import_set(args[1]);
          String out = args.size() >= 3 && !starts_with(args[2], _("--"))
            ? args[2]
            : settings.gameSettingsFor(*set->game).images_export_filename;
          String path = _(".");
          size_t pos = out.find_last_of(_("/\\"));
          if (pos != String::npos) {
            path = out.substr(0, pos);
            if (!wxDirExists(path)) wxMkdir(path);
            path += _("/x");
            out = out.substr(pos + 1);
          }
          export_image(set, set->cards, path, out, CONFLICT_NUMBER_OVERWRITE);
          return EXIT_SUCCESS;
        } else if (args[0] == _("--export")) {
          if (args.size() < 2) {
            throw Error(_("No export template specified for --export"));
          } else if (args.size() < 3) {
            throw Error(_("No input set file specified for --export"));
          }
          String export_template = args[1];
          ExportTemplateP exp = ExportTemplate::byName(export_template);
          SetP set = import_set(args[2]);
          String out = args.size() >= 4 ? args[3] : _("");
          ScriptValueP result = export_set(set, set->cards, exp, out);
          if (out.empty()) {
            cli << result->toString();
          }
          return EXIT_SUCCESS;
        } else {
          handle_error(_("Invalid command line argument:\n") + arg);
        }
      }

      auto* wnd = new gui_qt::WelcomeWindow();
      show_and_run(app, wnd);
      return EXIT_SUCCESS;
    } CATCH_ALL_ERRORS(true);
    log_line("Unhandled error during Qt backend startup.");
    cli.print_pending_errors();
    return EXIT_FAILURE;
  }
};

std::unique_ptr<Backend> createBackend() {
  return std::make_unique<QtBackend>();
}

} // namespace gui
