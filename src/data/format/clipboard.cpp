//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) Twan van Laarhoven and the other MSE developers          |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ---------------------------------------------------------------------------- : Includes

#include <util/prec.hpp>
#include <data/format/clipboard.hpp>
#include <data/format/formats.hpp>
#include <data/card.hpp>
#include <data/set.hpp>
#include <data/game.hpp>
#include <data/stylesheet.hpp>
#include <data/keyword.hpp>
#include <util/io/package.hpp>
#include <script/scriptable.hpp>
#include <wx/sstream.h>
#include <cstring>

// ---------------------------------------------------------------------------- : Clipboard serialization

/// Serialize an object to a string, clipboard_package will be set to the given package.
template <typename T>
String serialize_for_clipboard(Package& package, T& object) {
  wxStringOutputStream stream;
  Writer writer(stream, file_version_clipboard);
  WITH_DYNAMIC_ARG(clipboard_package, &package);
    writer.handle(object);
  return stream.GetString();
}

template <typename T>
void deserialize_from_clipboard(T& object, Package& package, const String& data) {
  wxStringInputStream stream = {data};
  Reader reader(stream, nullptr, _("clipboard"));
  WITH_DYNAMIC_ARG(clipboard_package, &package);
    reader.handle_greedy(object);
}

// ---------------------------------------------------------------------------- : SerializedClipboardDataObject

SerializedClipboardDataObject::SerializedClipboardDataObject(const wxDataFormat& format)
  : wxDataObjectSimple(format)
{}

void SerializedClipboardDataObject::SetText(const String& text) {
  this->text = text;
}

const String& SerializedClipboardDataObject::GetText() const {
  return text;
}

size_t SerializedClipboardDataObject::GetDataSize() const {
  wxCharBuffer buffer = text.utf8_str();
  return strlen(buffer.data()) + 1;
}

bool SerializedClipboardDataObject::GetDataHere(void* buf) const {
  wxCharBuffer buffer = text.utf8_str();
  memcpy(buf, buffer.data(), strlen(buffer.data()) + 1);
  return true;
}

bool SerializedClipboardDataObject::SetData(size_t len, const void* buf) {
  if (!buf) return false;
  const char* data = static_cast<const char*>(buf);
  if (len > 0 && data[len - 1] == '\0') {
    --len;
  }
  text = String::FromUTF8(data, len);
  return true;
}

// ---------------------------------------------------------------------------- : CardDataObject

/// A wrapped cards for storing on the clipboard
struct WrappedCards {
  Game*         expected_game;
  String        game_name;
  vector<CardP> cards;
  
  DECLARE_REFLECTION();
};

IMPLEMENT_REFLECTION(WrappedCards) {
  REFLECT(game_name);
  if (game_name == expected_game->name()) {
    WITH_DYNAMIC_ARG(game_for_reading, expected_game);
    REFLECT(cards);
  }
}


wxDataFormat CardsDataObject::format = _("application/x-mse-cards");

CardsDataObject::CardsDataObject(const SetP& set, const vector<CardP>& cards)
  : SerializedClipboardDataObject(format)
{
  // set the stylesheet, so when deserializing we know whos style options we are reading
  bool* has_styling = new bool[cards.size()];
  for (size_t i = 0 ; i < cards.size() ; ++i) {
    has_styling[i] = cards[i]->has_styling && !cards[i]->stylesheet;
    if (has_styling[i]) {
      cards[i]->stylesheet = set->stylesheet;
    }
  }
  WrappedCards data = { set->game.get(), set->game->name(), cards };
  SetText(serialize_for_clipboard(*set, data));
  // restore cards
  for (size_t i = 0 ; i < cards.size() ; ++i) {
    if (has_styling[i]) {
      cards[i]->stylesheet = StyleSheetP();
    }
  }
  SetFormat(format);
  delete [] has_styling;
}

CardsDataObject::CardsDataObject()
  : SerializedClipboardDataObject(format)
{}

bool CardsDataObject::getCards(const SetP& set, vector<CardP>& out) {
  WrappedCards data = { set->game.get(), set->game->name() };
  deserialize_from_clipboard(data, *set, GetText());
  if (data.cards.empty()) return false;
  if (data.game_name == set->game->name()) {
    // Cards are from the same game
    out = data.cards;
    return true;
  } else {
    return false;
  }
}

// ---------------------------------------------------------------------------- : KeywordDataObject

/// A wrapped keyword for storing on the clipboard
struct WrappedKeyword {
  Game*    expected_game;
  String   game_name;
  KeywordP keyword;
  
  DECLARE_REFLECTION();
};

IMPLEMENT_REFLECTION(WrappedKeyword) {
  REFLECT(game_name);
  if (game_name == expected_game->name()) {
    WITH_DYNAMIC_ARG(game_for_reading, expected_game);
    REFLECT(keyword);
  }
}


wxDataFormat KeywordDataObject::format = _("application/x-mse-keyword");

KeywordDataObject::KeywordDataObject(const SetP& set, const KeywordP& keyword)
  : SerializedClipboardDataObject(format)
{
  WrappedKeyword data = { set->game.get(), set->game->name(), keyword };
  SetText(serialize_for_clipboard(*set, data));
}

KeywordDataObject::KeywordDataObject()
  : SerializedClipboardDataObject(format)
{}

KeywordP KeywordDataObject::getKeyword(const SetP& set) {
  KeywordP keyword(new Keyword());
  WrappedKeyword data = { set->game.get(), set->game->name(), keyword};
  deserialize_from_clipboard(data, *set, GetText());
  if (data.game_name != set->game->name()) return KeywordP(); // Keyword is from a different game
  else                                     return keyword;
}

// ---------------------------------------------------------------------------- : Card on clipboard

CardsOnClipboard::CardsOnClipboard(const SetP& set, const String id, const vector<CardP>& cards) {
  Add(new CardsDataObject(set, id, cards), true);

  if (!cards.empty()) {
    String text;
    for (size_t i = 0; i < cards.size(); ++i) {
      if (i > 0) text += _("\n");
      text += cards[i]->identification();
    }
  // Conversion to bitmap format
    if (cards.size() == 1) {
      Add(new wxBitmapDataObject(export_bitmap(set, cards[0])));
    }
}
