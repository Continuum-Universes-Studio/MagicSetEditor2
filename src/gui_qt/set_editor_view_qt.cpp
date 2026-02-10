//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

#include "set_editor_view_qt.hpp"

#include "set_editor_controller_qt.hpp"

#include <data/card.hpp>
#include <data/game.hpp>
#include <data/set.hpp>

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace gui_qt {
namespace {

QString toQString(const String& value) {
  return QString::fromStdWString(value.ToStdWstring());
}

} // namespace

SetEditorViewQt::SetEditorViewQt(SetEditorControllerQt& controller, QWidget* parent)
  : QWidget(parent)
  , controller_(controller)
  , title_label_(new QLabel(this))
  , details_label_(new QLabel(this))
  , cards_list_(new QListWidget(this)) {
  auto* root = new QVBoxLayout(this);

  auto* toolbar = new QHBoxLayout();
  auto* open_button = new QPushButton(QStringLiteral("Open Set…"), this);
  toolbar->addWidget(open_button);
  toolbar->addStretch(1);
  root->addLayout(toolbar);

  title_label_->setTextFormat(Qt::RichText);
  root->addWidget(title_label_);

  details_label_->setWordWrap(true);
  root->addWidget(details_label_);

  cards_list_->setSelectionMode(QAbstractItemView::SingleSelection);
  root->addWidget(cards_list_, 1);

  QObject::connect(open_button, &QPushButton::clicked, this, [this]() { onOpenSet(); });
}

void SetEditorViewQt::refresh() {
  cards_list_->clear();

  const SetP& set = controller_.set();
  if (!set) {
    title_label_->setText(QStringLiteral("<h2>Set Editor</h2>"));
    details_label_->setText(QStringLiteral("No set loaded. Use <b>Open Set…</b> to load a .mse-set file."));
    return;
  }

  title_label_->setText(QStringLiteral("<h2>%1</h2>").arg(toQString(set->identification())));

  const QString game_name = set->game ? toQString(set->game->name()) : QStringLiteral("(unknown)");
  details_label_->setText(
    QStringLiteral("Game: %1\nCards: %2\nFile: %3")
      .arg(game_name)
      .arg(set->cards.size())
      .arg(controller_.currentPath()));

  for (const CardP& card : set->cards) {
    cards_list_->addItem(toQString(card->identification()));
  }
}

void SetEditorViewQt::onOpenSet() {
  QString error_message;
  if (!controller_.openSetWithDialog(this, error_message)) {
    if (!error_message.isEmpty()) {
      QMessageBox::critical(this, QStringLiteral("Open Set"), error_message);
    }
    return;
  }

  refresh();
}

} // namespace gui_qt
