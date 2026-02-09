//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#include <gui/backend.hpp>

#include <gui_qt/windows.hpp>

#include <QApplication>
#include <QDateTime>
#include <QMessageLogContext>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

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

QString to_qstring(const char* arg) {
  return arg ? QString::fromLocal8Bit(arg) : QString();
}

enum class StartupRoute {
  ShowWelcome,
  OpenSymbolFile,
  OpenSetFile,
  OpenInstaller,
  ShowHelp,
};

StartupRoute route_for_argument(const QString& argument) {
  if (argument.endsWith(".mse-symbol", Qt::CaseInsensitive)) return StartupRoute::OpenSymbolFile;
  if (argument.endsWith(".mse-set", Qt::CaseInsensitive) || argument.endsWith(".mse", Qt::CaseInsensitive) ||
      argument.endsWith(".set", Qt::CaseInsensitive)) {
    return StartupRoute::OpenSetFile;
  }
  if (argument.endsWith(".mse-installer", Qt::CaseInsensitive)) return StartupRoute::OpenInstaller;
  if (argument == "--help" || argument == "-?" || argument == "-h") return StartupRoute::ShowHelp;
  return StartupRoute::ShowWelcome;
}

int show_help(const char* executable_name) {
  log_line(std::string("Help requested for executable: ") + (executable_name ? executable_name : "magicseteditor"));
  return EXIT_SUCCESS;
}

int run_route(QApplication& app, StartupRoute route, const QString& argument, const char* executable_name) {
  if (route == StartupRoute::ShowHelp) {
    return show_help(executable_name);
  }

  QWidget* window = nullptr;
  switch (route) {
    case StartupRoute::ShowWelcome:
      window = new gui_qt::WelcomeWindow();
      break;
    case StartupRoute::OpenSymbolFile:
      window = new gui_qt::SymbolWindow(argument);
      break;
    case StartupRoute::OpenSetFile:
      window = new gui_qt::SetWindow(argument);
      break;
    case StartupRoute::OpenInstaller: {
      auto* dialog = new gui_qt::PackagesWindow(argument);
      dialog->show();
      window = dialog;
      break;
    }
    case StartupRoute::ShowHelp:
      return EXIT_SUCCESS;
  }

  if (!window) {
    return EXIT_FAILURE;
  }

  window->show();
  return app.exec();
}

} // namespace

class QtBackend final : public Backend {
public:
  int run(int argc, char** argv) override {
    log_line("Starting Qt backend.");
    qInstallMessageHandler(qt_message_handler);
    QApplication app(argc, argv);

    const QString argument = argc > 1 ? to_qstring(argv[1]) : QString();
    const StartupRoute route = route_for_argument(argument);
    return run_route(app, route, argument, argv[0]);
  }
};

std::unique_ptr<Backend> createBackend() {
  return std::make_unique<QtBackend>();
}

} // namespace gui
