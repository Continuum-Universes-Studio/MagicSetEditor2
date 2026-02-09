//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#include "windows.hpp"

#include <QLabel>
#include <QVBoxLayout>

namespace gui_qt {

namespace {
QWidget* buildPlaceholder(const QString& title, const QString& message, QWidget* parent = nullptr) {
  auto* widget = new QWidget(parent);
  auto* layout = new QVBoxLayout(widget);
  layout->addWidget(new QLabel(QString("<h2>%1</h2>").arg(title), widget));
  layout->addWidget(new QLabel(message, widget));
  layout->addStretch(1);
  return widget;
}
} // namespace

WelcomeWindow::WelcomeWindow() {
  setWindowTitle("Magic Set Editor - Welcome");
  setCentralWidget(buildPlaceholder(
    "Welcome",
    "Qt6 backend placeholder. The full welcome experience is still implemented in wxWidgets.",
    this));
  resize(640, 480);
}

SetWindow::SetWindow(const QString& set_path) {
  setWindowTitle("Magic Set Editor - Set");
  const QString message = set_path.isEmpty()
    ? "Qt6 backend placeholder for the Set window."
    : QString("Qt6 backend placeholder for the Set window.\nSet file: %1").arg(set_path);
  setCentralWidget(buildPlaceholder("Set Editor", message, this));
  resize(900, 600);
}

SymbolWindow::SymbolWindow(const QString& symbol_path) {
  setWindowTitle("Magic Set Editor - Symbol Editor");
  const QString message = symbol_path.isEmpty()
    ? "Qt6 backend placeholder for the Symbol Editor."
    : QString("Qt6 backend placeholder for the Symbol Editor.\nSymbol file: %1").arg(symbol_path);
  setCentralWidget(buildPlaceholder("Symbol Editor", message, this));
  resize(700, 500);
}

PackagesWindow::PackagesWindow(const QString& installer_path, QWidget* parent)
  : QDialog(parent) {
  setWindowTitle("Magic Set Editor - Package Installer");
  auto* layout = new QVBoxLayout(this);
  layout->addWidget(new QLabel("Qt6 backend placeholder for the package installer.", this));
  if (!installer_path.isEmpty()) {
    layout->addWidget(new QLabel(QString("Installer: %1").arg(installer_path), this));
  }
  resize(600, 300);
}

} // namespace gui_qt
