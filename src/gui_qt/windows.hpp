//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

#include <QDialog>
#include <QMainWindow>
#include <QString>

namespace gui_qt {

class WelcomeWindow final : public QMainWindow {
public:
  WelcomeWindow();
};

class SetWindow final : public QMainWindow {
public:
  explicit SetWindow(const QString& set_path);
};

class SymbolWindow final : public QMainWindow {
public:
  explicit SymbolWindow(const QString& symbol_path = QString());
};

class PackagesWindow final : public QDialog {
public:
  explicit PackagesWindow(const QString& installer_path, QWidget* parent = nullptr);
};

} // namespace gui_qt
