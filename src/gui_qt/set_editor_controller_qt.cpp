//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#include "set_editor_controller_qt.hpp"

#include <data/format/formats.hpp>
#include <data/settings.hpp>
#include <util/error.hpp>

#include <QFileDialog>

namespace gui_qt {
namespace {

String toMseString(const QString& value) {
  return wxString::FromUTF8(value.toUtf8().constData());
}

QString toQString(const String& value) {
  return QString::fromStdWString(value.ToStdWstring());
}

} // namespace

SetEditorControllerQt::SetEditorControllerQt() = default;

bool SetEditorControllerQt::openSetWithDialog(QWidget* parent, QString& error_message) {
  QString start_dir;
  if (!settings.default_set_dir.empty()) {
    start_dir = toQString(settings.default_set_dir);
  }

  const QString selected_path = QFileDialog::getOpenFileName(
    parent,
    QStringLiteral("Open Set"),
    start_dir,
    QStringLiteral("Set files (*.mse-set);;All files (*)"));

  if (selected_path.isEmpty()) {
    return false;
  }

  return loadSetFromPath(selected_path, error_message);
}

bool SetEditorControllerQt::loadSetFromPath(const QString& set_path, QString& error_message) {
  try {
    const String file = toMseString(set_path);
    set_ = import_set(file);
    current_path_ = set_path;
    settings.addRecentFile(file);
    error_message.clear();
    return true;
  } CATCH_ALL_ERRORS(false)

  MessageType type = MESSAGE_NONE;
  String message;
  if (get_queued_message(type, message)) {
    error_message = toQString(message);
  } else {
    error_message = QStringLiteral("Failed to open set.");
  }

  return false;
}

} // namespace gui_qt
