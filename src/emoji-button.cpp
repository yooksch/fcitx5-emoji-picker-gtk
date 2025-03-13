#include "emoji-button.hpp"

EmojiButton::EmojiButton() {
  auto context = get_style_context();
  Gdk::RGBA color;
  color.set_rgba(0.3f, 0.3f, 0.3f, 1.f);

  context->lookup_color("theme_selected_bg_color", color);

  auto provider = Gtk::CssProvider::create();
  provider->load_from_data("button { transition: none; } button.highlighted { background-color: " + color.to_string() + "; } button.hidden { opacity: 0; }");

  auto screen = Gdk::Screen::get_default();
  context->add_provider_for_screen(screen, provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

void EmojiButton::highlight() {
  get_style_context()->add_class("highlighted");
}

void EmojiButton::remove_highlight() {
  get_style_context()->remove_class("highlighted");
}

void EmojiButton::show() {
  get_style_context()->remove_class("hidden");
}

void EmojiButton::hide() {
  get_style_context()->add_class("hidden");
}

void EmojiButton::set_emoji(Emoji emoji) {
  set_label(emoji.code);
  m_emoji = emoji;
}

Emoji EmojiButton::get_emoji() {
  return m_emoji;
}
