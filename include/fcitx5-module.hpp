#pragma once

#include "window.hpp"
#include <fcitx/addonfactory.h>
#include <fcitx/addoninstance.h>
#include <fcitx/addonmanager.h>
#include <fcitx/instance.h>

constexpr int KEY_ESCAPE = 9;
constexpr int KEY_RETURN = 36;
constexpr int KEY_BACKSPACE = 22;
constexpr int KEY_TAB = 23;
constexpr int KEY_ARROW_LEFT = 113;
constexpr int KEY_ARROW_RIGHT = 114;
constexpr int KEY_ARROW_UP = 111;
constexpr int KEY_ARROW_DOWN = 116;

FCITX_CONFIGURATION(
  Fcitx5EmojiPickerConfig,
  fcitx::KeyListOption triggerKey {
    this,
    "TriggerKey",
    "Trigger Key",
    { fcitx::Key("Control+Alt+period") },
    fcitx::KeyListConstrain()
  };
);

class Fcitx5EmojiPickerModule : public fcitx::AddonInstance {
  public:
    Fcitx5EmojiPickerModule(fcitx::Instance* instance);
    void process_key_event(fcitx::KeyEvent& keyEvent);
    void activate(fcitx::InputContextEvent& event);
    void deactivate();

    void set_window(Window* window);
  private:
    static constexpr char CONFIG_FILE[] = "conf/emoji-picker-gtk.conf";

    fcitx::Instance* m_instance;
    std::vector<std::unique_ptr<fcitx::HandlerTableEntry<fcitx::EventHandler>>> m_events;
    bool m_active = false;
    Fcitx5EmojiPickerConfig m_config;
    Window* m_window = nullptr;

    void reloadConfig() override;
    void setConfig(const fcitx::RawConfig& config) override;
    const fcitx::Configuration* getConfig() const override;
};

class Fcitx5EmojiPickerModuleFactory : public fcitx::AddonFactory {
  public:
    fcitx::AddonInstance* create(fcitx::AddonManager* manager) override;
};
