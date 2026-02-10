//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#include "windows.hpp"

#include "set_editor_controller_qt.hpp"
#include "set_editor_view_qt.hpp"

#include <QLabel>
#include <QMessageBox>
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
  // TODO(Qt welcome route): Replace this placeholder with the real welcome workflow and recent file list.
  setCentralWidget(buildPlaceholder(
    "Welcome",
    "Qt6 backend placeholder. The full welcome experience is still implemented in wxWidgets.",
    this));
  resize(640, 480);
}

SetWindow::SetWindow(const QString& set_path)
  : controller_(new SetEditorControllerQt()) {
  setWindowTitle("Magic Set Editor - Set");

  auto* view = new SetEditorViewQt(*controller_, this);
  setCentralWidget(view);

  if (!set_path.isEmpty()) {
    QString error_message;
    if (!controller_->loadSetFromPath(set_path, error_message) && !error_message.isEmpty()) {
      QMessageBox::critical(this, QStringLiteral("Open Set"), error_message);
    }
  }

  view->refresh();
  resize(900, 600);
}

SymbolWindow::SymbolWindow(const QString& symbol_path) {
  setWindowTitle("Magic Set Editor - Symbol Editor");
  // TODO(Qt symbol route): Replace this placeholder with a functional symbol editor integrated with core symbol/font logic.
  const QString message = symbol_path.isEmpty()
    ? "Qt6 backend placeholder for the Symbol Editor."
    : QString("Qt6 backend placeholder for the Symbol Editor.\nSymbol file: %1").arg(symbol_path);
  setCentralWidget(buildPlaceholder("Symbol Editor", message, this));
  resize(700, 500);
}

PackagesWindow::PackagesWindow(const QString& installer_path, QWidget* parent)
  : QDialog(parent) {
  setWindowTitle("Magic Set Editor - Package Installer");
  // TODO(Qt installer route): Replace this placeholder with a Qt package installer UI using existing installer logic.
  auto* layout = new QVBoxLayout(this);
  layout->addWidget(new QLabel("Qt6 backend placeholder for the package installer.", this));
  if (!installer_path.isEmpty()) {
    layout->addWidget(new QLabel(QString("Installer: %1").arg(installer_path), this));
  }
  resize(600, 300);
}

} // namespace gui_qt
