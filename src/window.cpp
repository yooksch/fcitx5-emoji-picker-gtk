#include "window.hpp"
#include "fcitx5-module.hpp"
#include <algorithm>
#include <climits>
#include <cmath>
#include <gtk-layer-shell.h>
#include <string>
#include <unordered_set>

Window::Window() {
  set_title("Fcitx5 Emoji Picker");
  set_resizable(false);

  // setup gtk layer shell
  auto window = this->gobj();
  gtk_layer_init_for_window(window);
  gtk_layer_set_layer(window, GTK_LAYER_SHELL_LAYER_OVERLAY);

  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_LEFT, true);
  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_TOP, true);

  // create and initialize UI components
  m_box.set_border_width(10);
  m_box.set_orientation(Gtk::ORIENTATION_VERTICAL);
  m_box.set_spacing(6);

  {
    m_search.set_placeholder_text("Search");
    m_search.set_max_length(INT_MAX);

    auto context = m_search.get_style_context();
    Gdk::RGBA color;
    color.set_rgba(0.3f, 0.3f, 0.3f);

    context->lookup_color("theme_selected_bg_color", color);
    
    auto css_provider = Gtk::CssProvider::get_default();
    css_provider->load_from_data("entry { border-color: " + color.to_string() +" }");

    m_search.get_style_context()->add_provider(css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    m_box.pack_start(m_search, Gtk::PACK_SHRINK);
  }

  m_grid.set_row_spacing(3);
  m_grid.set_column_spacing(3);

  for (int r = 0; r < EMOJI_GRID_HEIGHT; r++) {
    for (int c = 0; c < EMOJI_GRID_WIDTH; c++) {
      auto button = Gtk::make_managed<EmojiButton>();
      button->set_hexpand();
      button->set_vexpand();

      // save row and column
      int row = r;
      int column = c;
      button->signal_clicked().connect([=, this]() {
        set_cursor_pos(column, row);
        commit_string(m_emoji_buttons[row * EMOJI_GRID_WIDTH * column]->get_emoji().code);
      });

      m_grid.attach(*button, c, r);
      m_emoji_buttons.push_back(button);
    }
  }

  m_box.pack_start(m_grid, Gtk::PACK_EXPAND_WIDGET);

  add(m_box);
  show_all_children();

  // Init emojis
  update_search_results();
  update_emoji_grid();
  set_cursor_pos(0, 0);

  // Hide after intialization is complete
  Glib::signal_idle().connect([this]() {
    hide();
    return false;
  });
}

void Window::handle_fcitx_key_event(fcitx::KeyEvent& event) {
  if (event.isRelease()) return;

  auto key = event.key().code();

  auto cursor_pos = m_search.get_position();
  switch (key) {
    case KEY_ESCAPE:
      break;
    case KEY_RETURN: {
      commit_string(m_emoji_buttons[m_cursor_y * EMOJI_GRID_WIDTH + m_cursor_x]->get_emoji().code);
      break;}
    case KEY_BACKSPACE:
      if (cursor_pos > 0) {
        m_search.set_text(m_search.get_text().erase(cursor_pos - 1, 1));
        m_search.set_position(cursor_pos - 1);
        update_search_results();
        update_emoji_grid();
      }
      break;
    case KEY_ARROW_LEFT:
      move_selection(-1, 0);
      break;
    case KEY_TAB:
    case KEY_ARROW_RIGHT:
      move_selection(1, 0);
      break;
    case KEY_ARROW_DOWN:
      move_selection(0, 1);
      break;
    case KEY_ARROW_UP:
      move_selection(0, -1);
      break;
    default:
      int ksym = event.key().sym();
      if (ksym >= 32 && ksym <= 126) {
        m_search.insert_text(Glib::ustring(1, static_cast<char>(ksym)), 1, cursor_pos);
        m_search.set_position(cursor_pos + 1);
        update_search_results();
        update_emoji_grid();
      }
      break;
  }
}

void Window::set_cursor_pos(int x, int y) {
  m_emoji_buttons[m_cursor_y * EMOJI_GRID_WIDTH + m_cursor_x]->remove_highlight();
  m_emoji_buttons[y * EMOJI_GRID_WIDTH + x]->highlight();

  m_cursor_x = x;
  m_cursor_y = y;
}

void Window::move_selection(int dx, int dy) {
  int new_x = m_cursor_x + dx;
  int new_y = m_cursor_y + dy;

  if (new_x >= EMOJI_GRID_WIDTH) {
    new_y++;
    new_x = 0;
  } else if (new_x < 0) {
    new_y--;
    new_x = EMOJI_GRID_WIDTH - 1;
  }

  if (new_y < 0) {
    if (m_scroll > 0) {
      m_scroll--;
      update_emoji_grid();
    }
    new_y = 0;
  } else if (new_y >= EMOJI_GRID_HEIGHT && m_scroll + 1 < m_scroll_max) {
    m_scroll++;
    new_y = EMOJI_GRID_HEIGHT - 1;
    update_emoji_grid();
  }

  set_cursor_pos(new_x, new_y);
}

void Window::update_search_results() {
  set_cursor_pos(0, 0);
  m_scroll = 0;
  m_search_results.clear();

  if (m_search.get_text_length() > 0) {
    enum SearchMode {
      StartsWith,
      Contains,
      Equals
    };

    std::unordered_set<Emoji, EmojiHash> results_set;
    auto term = m_search.get_text().lowercase();
    auto search = [&](SearchMode mode) {
      for (const Emoji& emoji : emojis) {
        if (results_set.find(emoji) != results_set.end()) continue;
        switch (mode) {
          case Contains:
            if (emoji.name.find(term) != std::string::npos) {
              results_set.emplace(emoji);
              m_search_results.emplace_back(emoji);
            }
            break;
          case StartsWith:
            if (emoji.name.size() >= term.size() && emoji.name.substr(0, term.size()) == term) {
              results_set.emplace(emoji);
              m_search_results.emplace_back(emoji);
            }
            break;
          case Equals:
            if (emoji.name.compare(term) == 0) {
              results_set.emplace(emoji);
              m_search_results.emplace_back(emoji);
            }
            break;
        }
      }
    };

    search(SearchMode::Equals);
    search(SearchMode::StartsWith);
    search(SearchMode::Contains);
  } else {
    for (const Emoji& emoji : emojis)
      m_search_results.emplace_back(emoji);
  }

  m_scroll_max = static_cast<int>(std::ceil(m_search_results.size() / (EMOJI_GRID_WIDTH - 1)));
}

void Window::update_emoji_grid() {
  Glib::signal_idle().connect([this]() {
    int offset = m_scroll * EMOJI_GRID_WIDTH;
    for (size_t i = 0; i < m_emoji_buttons.size(); i++) {
      if (m_search_results.size() > offset + i) {
        m_emoji_buttons[i]->show();
        m_emoji_buttons[i]->set_emoji(m_search_results[offset + i]);
      } else {
        m_emoji_buttons[i]->hide();
      }
    }

    return false;
  });
}

void Window::set_window_pos(const fcitx::Rect& rect) {
  auto window = gobj();
  auto scale_factor = gtk_widget_get_scale_factor(GTK_WIDGET(window));

  if (rect.left() == 0 && rect.top() == 0) {
    gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_LEFT, false);
    gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_TOP, false);
    return;
  }

  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_LEFT, true);
  gtk_layer_set_anchor(window, GTK_LAYER_SHELL_EDGE_TOP, true);
  gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_LEFT, rect.left() * scale_factor);
  gtk_layer_set_margin(window, GTK_LAYER_SHELL_EDGE_TOP, rect.top() * scale_factor);
}

void Window::reset() {
  m_search.set_text("");
  m_scroll = 0;
  set_cursor_pos(0, 0);
  update_search_results();
  update_emoji_grid();
}
