//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#pragma once

#include <QWidget>

namespace gui_qt {

class SetEditorControllerQt;
class QLabel;
class QListWidget;

class SetEditorViewQt final : public QWidget {
public:
  explicit SetEditorViewQt(SetEditorControllerQt& controller, QWidget* parent = nullptr);

  void refresh();

private:
  void onOpenSet();

  SetEditorControllerQt& controller_;
  QLabel* title_label_;
  QLabel* details_label_;
  QListWidget* cards_list_;
};

} // namespace gui_qt
