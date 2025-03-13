#pragma once

#include <fcitx/event.h>
#include <functional>
#include <gtkmm.h>
#include <gtk-layer-shell.h>
#include "emoji-button.hpp"
#include "emojis.hpp"
#include <fcitx-utils/rect.h>

class Window : public Gtk::Window {
  public:
    std::function<void (Glib::ustring)> commit_string = [](Glib::ustring _) { };

    Window();
    void handle_fcitx_key_event(fcitx::KeyEvent& event);
    void set_cursor_pos(int x, int y);
    void move_selection(int dx, int dy);
    void update_search_results();
    void update_emoji_grid();
    void set_window_pos(const fcitx::Rect& rect);
    void reset();
  private:
    static constexpr int EMOJI_GRID_WIDTH = 5;
    static constexpr int EMOJI_GRID_HEIGHT = 3;

    Gtk::Box m_box;
    Gtk::Entry m_search;
    Gtk::Grid m_grid;
    std::vector<EmojiButton*> m_emoji_buttons;
    std::vector<Emoji> m_search_results;
    int m_cursor_x = 0;
    int m_cursor_y = 0;
    int m_scroll = 0;
    int m_scroll_max = 0;
};
