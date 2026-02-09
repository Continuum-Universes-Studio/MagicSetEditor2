#pragma once

#include <util/prec.hpp>

class Game;
DECLARE_POINTER_TYPE(StyleSheet);

namespace gui_core {

class MissingStyleSheetPresenter {
public:
  virtual ~MissingStyleSheetPresenter() = default;
  virtual StyleSheetP selectMissingStyleSheet(const Game& game, const String& failed_name) = 0;
};

StyleSheetP select_stylesheet(const Game& game, const String& failed_name, MissingStyleSheetPresenter* presenter);

} // namespace gui_core
