#include "fcitx5-module.hpp"

#include <fcitx-config/iniparser.h>
#include <fcitx/event.h>
#include <gtkmm/application.h>
#include <thread>

Fcitx5EmojiPickerModule::Fcitx5EmojiPickerModule(fcitx::Instance* instance) {
  // Listen for the trigger key and open the UI if needed
  m_events.emplace_back(instance->watchEvent(
    fcitx::EventType::InputContextKeyEvent,
    fcitx::EventWatcherPhase::Default,
    [this](fcitx::Event& event) {
      auto& key_event = static_cast<fcitx::KeyEvent&>(event);

      if (key_event.isRelease()) return;
      if (!key_event.key().checkKeyList(*m_config.triggerKey)) return;

      key_event.filterAndAccept();
      activate(key_event);
    }
  ));

  // Handle key presses
  m_events.emplace_back(instance->watchEvent(
    fcitx::EventType::InputContextKeyEvent,
    fcitx::EventWatcherPhase::PreInputMethod,
    [this](fcitx::Event& event) {
      if (!m_active) return;

      auto& key_event = static_cast<fcitx::KeyEvent&>(event);

      key_event.filter();
      process_key_event(key_event);
    }
  ));

  // Register deactivate events
  m_events.emplace_back(instance->watchEvent(
    fcitx::EventType::InputContextFocusOut,
    fcitx::EventWatcherPhase::Default,
    [this](fcitx::Event& _) {
      deactivate();
    }
  ));

  m_events.emplace_back(instance->watchEvent(
    fcitx::EventType::InputContextReset,
    fcitx::EventWatcherPhase::Default,
    [this](fcitx::Event& _) {
      deactivate();
    }
  ));

  reloadConfig();
}

void Fcitx5EmojiPickerModule::process_key_event(fcitx::KeyEvent& key_event) {
  if (key_event.key().isModifier()) return;

  if (key_event.key().code() == KEY_ESCAPE) {
    key_event.accept();

    deactivate();
    return;
  }

  if (!m_window) return;
  key_event.accept();

  auto context = key_event.inputContext();
  if (context)
    m_window->set_window_pos(context->cursorRect());

  m_window->handle_fcitx_key_event(key_event);
}

void Fcitx5EmojiPickerModule::activate(fcitx::InputContextEvent& event) {
  if (m_active) return;
  m_active = true;

  if (m_window) {
    auto context = event.inputContext();

    // Setup callback
    m_window->commit_string = [=](Glib::ustring text) {
      context->commitString(text);
    };

    m_window->set_window_pos(context->cursorRect());
    m_window->show_all();
  }
}


void Fcitx5EmojiPickerModule::deactivate() {
  if (!m_active) return;
  m_active = false;

  if (m_window) {
    m_window->hide();
    m_window->reset();
  }
}

void Fcitx5EmojiPickerModule::set_window(Window* window) {
  m_window = window;
}

void Fcitx5EmojiPickerModule::reloadConfig() {
  fcitx::readAsIni(m_config, CONFIG_FILE);
}

void Fcitx5EmojiPickerModule::setConfig(const fcitx::RawConfig& rawConfig) {
  m_config.load(rawConfig, true);
  safeSaveAsIni(m_config, CONFIG_FILE);
}

const fcitx::Configuration* Fcitx5EmojiPickerModule::getConfig() const {
  return &m_config;
}

fcitx::AddonInstance* Fcitx5EmojiPickerModuleFactory::create(fcitx::AddonManager* manager) {
  auto mod = new Fcitx5EmojiPickerModule(manager->instance());

  // create ui thread
  std::thread([=]() {
      // Create gtk application
      auto app = Gtk::Application::create("com.yooksch.fcitx5-emoji-picker-gtk");

      // prevent the app from quitting when no windows are visible
      app->hold();

      // Construct and initialize the window
      auto window = new Window();

      // Give the emoji picker module access to the window
      mod->set_window(window);

      // Run the window
      app->run(*window);
  }).detach();

  return mod;
}

FCITX_ADDON_FACTORY(Fcitx5EmojiPickerModuleFactory);
