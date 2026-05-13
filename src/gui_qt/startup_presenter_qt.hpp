#pragma once

#include <gui_core/startup_flow.hpp>

class QApplication;
class QWidget;

namespace gui_qt {

class QtStartupPresenter final : public gui_core::StartupPresenter {
public:
  explicit QtStartupPresenter(QApplication& app);

  int showWelcomeWindow() override;
  int showSymbolEditor(const String& file) override;
  int showSetEditor(const String& file) override;
  int showInstaller(const String& installer_file, InstallType install_type) override;

private:
  int showAndRun(QWidget* window);

  QApplication& app_;
};

} // namespace gui_qt
