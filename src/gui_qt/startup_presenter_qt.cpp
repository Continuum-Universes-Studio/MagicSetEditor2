#include "startup_presenter_qt.hpp"

#include <gui_qt/windows.hpp>

#include <QApplication>

namespace gui_qt {

QtStartupPresenter::QtStartupPresenter(QApplication& app) : app_(app) {}

int QtStartupPresenter::showAndRun(QWidget* window) {
  window->show();
  return app_.exec();
}

int QtStartupPresenter::showWelcomeWindow() {
  return showAndRun(new WelcomeWindow());
}

int QtStartupPresenter::showSymbolEditor(const String& file) {
  return showAndRun(new SymbolWindow(file.empty() ? QString() : QString::fromStdWString(file.ToStdWstring())));
}

int QtStartupPresenter::showSetEditor(const String& file) {
  return showAndRun(new SetWindow(QString::fromStdWString(file.ToStdWstring())));
}

int QtStartupPresenter::showInstaller(const String& installer_file, InstallType install_type) {
  (void)install_type;
  PackagesWindow wnd(QString::fromStdWString(installer_file.ToStdWstring()));
  wnd.exec();
  return EXIT_SUCCESS;
}

} // namespace gui_qt
