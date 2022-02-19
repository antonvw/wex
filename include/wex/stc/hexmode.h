////////////////////////////////////////////////////////////////////////////////
// Name:      hexmode.h
// Purpose:   Declaration of class wex::hexmode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/hexmode.h>
#include <wx/event.h>

namespace wex
{
class hexmode_line;

namespace factory
{
class stc;
};

/// Offers a hex mode.
class hexmode : public factory::hexmode
{
  friend hexmode_line;

public:
  /// Constructor.
  hexmode(
    /// stc to view in hexmode
    factory::stc* stc,
    /// bytes per line
    size_t bytes_per_line = 16);

  /// If hex mode is on, appends hex mode lines to stc component.
  /// The text should be normal ascii text, it is encoded while appending.
  void append_text(const std::string& text);

  /// Shows a control char dialog.
  void control_char_dialog(const std::string& caption);

  /// Deletes chars at current index at current line for
  /// both ascii and hex field.
  bool erase(int count = 1, int pos = -1);

  /// Returns the buffer.
  /// The buffer contains the normal text, without hex info.
  const auto& buffer() const { return m_buffer; }

  /// Returns info about current index,
  /// depending on which field is current.
  const std::string get_info();

  /// Asks for a byte offset goes to that byte.
  bool goto_dialog();

  /// Highlights the corresponding char for the other field
  /// for the current position.
  bool highlight_other();

  /// Highlights the corresponding char for the other field.
  bool highlight_other(int pos);

  /// Inserts text at position.
  /// Insert at ascii field or at hex field,
  /// at hex field you should provide the ascii
  /// hex codes, e.g. "30" inserts one byte space.
  bool insert(const std::string& text, int pos = -1);

  /// Replaces current line at current index (if pos -1) with char for
  /// both ascii and hex field. Otherwise at specified pos.
  bool replace(char c, int pos = -1);

  /// Replaces target with replacement text.
  /// This is only possible for hex the field,
  /// therefore the target start and target end should be within
  /// the hex field.
  /// Returns false if target outside area, or replacement
  /// text has invalid chars, or doc is readonly.
  bool replace_target(
    /// should contain hex codes (uppercase): 303AFF.
    const std::string& replacement,
    /// invokes SetText after replacing.
    bool settext = true);

  /// Sets hex mode.
  /// Returns true if mode is changed.
  bool set(bool on, bool use_modification_markers = true);

  /// Sets caret pos on stc, depending on event and
  /// where we are.
  void set_pos(const wxKeyEvent& event);

  /// Sets text, if hex mode is on.
  /// The text should be normal ascii text, it is encoded while appending.
  void set_text(const std::string text);

  /// Sync, set text with buffer.
  bool sync();

  /// Undo change, sets the buffer to the original buffer.
  void undo();

private:
  // Override methods.
  void activate() override;
  void deactivate() override;

  int m_goto = 0;

  std::string m_buffer, m_buffer_original;
};
}; // namespace wex
