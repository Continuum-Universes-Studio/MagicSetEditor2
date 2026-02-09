#pragma once

#include <util/prec.hpp>

#include <data/installer.hpp>

namespace gui_core {

enum class StartupRoute {
  ShowWelcome,
  OpenSymbolFile,
  OpenSetFile,
  OpenInstaller,
  RunScript,
  OpenSymbolEditor,
  CreateInstaller,
  ShowHelp,
  ShowVersion,
  RunCli,
  ExportImages,
  ExportSet,
  InvalidArgument,
};

struct StartupRequest {
  StartupRoute route = StartupRoute::ShowWelcome;
  vector<String> args;
  String argument;
  InstallType install_type;
};

class StartupPresenter {
public:
  virtual ~StartupPresenter() = default;
  virtual int showWelcomeWindow() = 0;
  virtual int showSymbolEditor(const String& file) = 0;
  virtual int showSetEditor(const String& file) = 0;
  virtual int showInstaller(const String& installer_file, InstallType install_type) = 0;
};

StartupRequest parse_startup_request(int argc, char** argv, InstallType default_install_type);
int run_startup_request(const StartupRequest& request, StartupPresenter& presenter, const char* executable_name);

} // namespace gui_core
