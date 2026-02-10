//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#include <gui/backend.hpp>

#include <cli/cli_main.hpp>
#include <cli/text_io_handler.hpp>
#include <data/format/formats.hpp>
#include <data/locale.hpp>
#include <data/settings.hpp>
#include <gui_core/startup_flow.hpp>
#include <gui_qt/select_stylesheet_qt.hpp>
#include <gui_qt/startup_presenter_qt.hpp>
#include <util/io/package_manager.hpp>
#include <util/spell_checker.hpp>

#include <QApplication>
#include <QDateTime>
#include <QMessageLogContext>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>

namespace gui {

namespace {
std::string resolve_log_dir() {
  const char* home = std::getenv("HOME");
#ifdef _WIN32
  if (!home) home = std::getenv("USERPROFILE");
  if (!home) home = std::getenv("APPDATA");
#endif
  if (!home || std::string(home).empty()) return ".";
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
      log_stream.open(log_dir + "/boot.log", std::ios::out | std::ios::app);
      if (log_stream.is_open()) {
        log_stream << "\n--- Magic Set Editor boot log (Qt6 backend) ---\n";
      }
    } catch (...) {
    }
  }
  return log_stream;
}

void log_line(const std::string& message) {
  auto& stream = boot_log();
  if (!stream.is_open()) return;
  const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  stream << std::string(std::ctime(&now)).substr(0, 24) << " | " << message << "\n";
}

const char* qt_log_level(QtMsgType type) {
  switch (type) {
    case QtDebugMsg: return "DEBUG";
    case QtInfoMsg: return "INFO";
    case QtWarningMsg: return "WARN";
    case QtCriticalMsg: return "CRITICAL";
    case QtFatalMsg: return "FATAL";
    default: return "UNKNOWN";
  }
}

void qt_message_handler(QtMsgType type, const QMessageLogContext& context, const QString& message) {
  std::string output = std::string("Qt ") + qt_log_level(type) + ": " + message.toStdString();
  if (context.file && context.line > 0) {
    output += " (" + std::string(context.file) + ":" + std::to_string(context.line) + ")";
  }
  log_line(output);
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
    qInstallMessageHandler(qt_message_handler);
    QApplication app(argc, argv);

    init_script_variables();
    init_file_formats();
    cli.init();
    package_manager.init();
    settings.read();
    the_locale = Locale::byName(settings.locale);
    gui_qt::install_missing_stylesheet_presenter();
    ScopedCleanup cleanup;

    try {
      gui_qt::QtStartupPresenter presenter(app);
      const gui_core::StartupRequest request =
        gui_core::parse_startup_request(argc, argv, settings.install_type);
      return gui_core::run_startup_request(request, presenter, argv[0]);
    } CATCH_ALL_ERRORS(true);
    cli.print_pending_errors();
    return EXIT_FAILURE;
  }
};

std::unique_ptr<Backend> createBackend() {
  return std::make_unique<QtBackend>();
}

} // namespace gui
