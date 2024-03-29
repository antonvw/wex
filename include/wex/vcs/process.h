////////////////////////////////////////////////////////////////////////////////
// Name:      process.h
// Purpose:   Declaration of class wex::process
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/process.h>
#include <wex/factory/window.h>

namespace wex
{
class frame;
class shell;

/// Offers a process, capturing execution output.
class process : public factory::process
{
public:
  // Static interface.

  /// Shows a config dialog, allowing you to set the command and folder.
  /// Returns dialog return code.
  static int config_dialog(const data::window& data = data::window());

  /// Returns the shell component
  /// (might be nullptr if prepare_output is not yet invoked).
  static auto* get_shell() { return m_shell; }

  /// Construct the shell component, and returns it.
  static shell* prepare_output(wxWindow* parent);

  // Other methods.

  /// Default constructor.
  process();

  // Virtual interface

  /// Shows stdout or stderr from system on the shell component.
  /// You can override this method to e.g. prepare a lexer on get_shell
  /// before calling this base method.
  virtual void show_output(const std::string& caption = std::string()) const;

  /// Override methods.

  /// See factory::process.
  /// Return value is false if process could not execute,
  /// or if config dialog was invoked and cancelled, or prepare_output
  /// not yet invoked.
  bool async_system(const process_data& data) override;

  /// See factory::process.
  int system(const process_data& data) override;

  /// Writes text to stdin of process.
  /// The response stdout is collected in the shell.
  bool write(const std::string& text) override;

  // Other methods.

  /// Returns the frame.
  auto* get_frame() { return m_frame; }

private:
  static inline shell* m_shell = nullptr;
  static std::string   m_working_dir_key;

  void set_frame();

  frame* m_frame;
};
}; // namespace wex
