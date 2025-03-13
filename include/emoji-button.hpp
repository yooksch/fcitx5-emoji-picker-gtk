#pragma once

#include "emojis.hpp"
#include <gtkmm.h>

class EmojiButton : public Gtk::Button {
  public:
    EmojiButton();
    void highlight();
    void remove_highlight();
    void show();
    void hide();
    void set_emoji(Emoji emoji);
    Emoji get_emoji();
  private:
    Emoji m_emoji;
};
