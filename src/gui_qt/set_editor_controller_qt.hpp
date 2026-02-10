//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

#include <QString>

#include <data/set.hpp>

class QWidget;

namespace gui_qt {

class SetEditorControllerQt final {
public:
  SetEditorControllerQt();

  bool openSetWithDialog(QWidget* parent, QString& error_message);
  bool loadSetFromPath(const QString& set_path, QString& error_message);

  const SetP& set() const {
    return set_;
  }

  QString currentPath() const {
    return current_path_;
  }

private:
  SetP set_;
  QString current_path_;
};

} // namespace gui_qt
