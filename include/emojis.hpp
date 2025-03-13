#pragma once

#include <gtkmm.h>

class Emoji {
  public:
    Glib::ustring name;
    Glib::ustring code;

    bool operator==(const Emoji& other) const {
      return code == other.code || name == other.name;
    }
};

struct EmojiHash {
  std::size_t operator()(const Emoji& emoji) const {
    return std::hash<std::string>()(emoji.code.raw());
  }
};

extern const Emoji emojis[4550];
