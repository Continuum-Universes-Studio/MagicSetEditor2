//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <util/application_paths.hpp>

#ifdef USE_QT6
  #include <QCoreApplication>
  #include <QDir>
  #include <QStandardPaths>
#else
  #include <wx/stdpaths.h>
#endif

namespace {
#ifdef USE_QT6
String to_mse_string(const QString& value) {
  return wxString::FromUTF8(value.toUtf8().constData());
}
#endif
}

String app_user_data_dir() {
#ifdef USE_QT6
  QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  if (dir.isEmpty()) {
    dir = QDir::homePath() + QStringLiteral("/.magicseteditor");
  }
  return to_mse_string(dir);
#else
  return wxStandardPaths::Get().GetUserDataDir();
#endif
}

String app_data_dir() {
#ifdef USE_QT6
  const QString app_dir = QCoreApplication::applicationDirPath();
  if (!app_dir.isEmpty()) {
    return to_mse_string(app_dir);
  }
  return to_mse_string(QDir::currentPath());
#else
  return wxStandardPaths::Get().GetDataDir();
#endif
}

