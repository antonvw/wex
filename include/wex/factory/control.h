////////////////////////////////////////////////////////////////////////////////
// Name:      data/control.h
// Purpose:   Declaration of wex::data::control
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/line-data.h>
#include <wex/factory/window.h>
#include <wx/validate.h>

#include <bitset>
#include <functional>

namespace wex::data
{
/// Offers user data to be used by ctrl classes (as listview, stc).
/// First you can set the data using col, command, line, etc.,
/// then call inject to perform the action.
/// You can set several items, inject prioritizes the actions.
class control : public line_data
{
public:
  /// Determine how flags value are set.
  enum action_t
  {
    INV, ///< sets value and flips result
    NOT, ///< remove this flag
    OR,  ///< add this flag
    SET, ///< set value
    XOR, ///< xor this flag
  };

  /// Returns col.
  const auto col() const { return line_data::col(); }

  /// Sets column.
  control& col(int col)
  {
    line_data::col(col);
    return *this;
  };

  /// Returns command.
  const auto& command() const { return line_data::command(); }

  /// Sets command.
  // This is necessary for code ui/ctags.cpp.
  control& command(const std::string& rhs)
  {
    line_data::command(rhs);
    return *this;
  }

  /// Sets is_ctag.
  // This is necessary for code ui/ctags.cpp.
  control& is_ctag(bool rhs)
  {
    line_data::is_ctag(rhs);
    return *this;
  }

  /// Returns find.
  const auto& find() const { return m_find; }

  /// Sets find.
  /// If not empty selects the text on that line (if line was specified)
  /// or finds text from begin (if line was 0) or end (line was -1).
  control& find(
    /// text to find
    const std::string& text,
    /// find flags to be used
    int find_flags = 0);

  /// Returns find flags.
  const auto find_flags() const { return m_find_flags; }

  /// Sets specified flags.
  /// This is used by the other data classes as generic
  /// method to operate on flags.
  template <std::size_t N>
  control& flags(
    const std::bitset<N>& flags,
    std::bitset<N>&       result,
    action_t              action = SET)
  {
    switch (action)
    {
      case INV:
        result = flags;
        result.flip();
        break;

      case NOT:
        result &= ~flags;
        break;

      case OR:
        result |= flags;
        break;

      case SET:
        result = flags;
        break;

      case XOR:
        result ^= flags;
        break;
    }
    return *this;
  };

  /// injects data.
  /// If there is a callback specified, injects current data in it:
  /// - if line available: goto line
  /// - if col available: goto col
  /// - if text not empty: finds text
  /// - if vi command not empty: executes vi command
  /// - if flags are set: sets flag on Control
  /// Returns true if data could be injected into the control.
  bool inject(
    /// callback to inject line number
    const std::function<bool(void)>& line = nullptr,
    /// callback to inject column number
    const std::function<bool(void)>& col = nullptr,
    /// callback to inject find text
    const std::function<bool(void)>& find = nullptr,
    /// callback to inject vi command
    const std::function<bool(void)>& command = nullptr) const;

  /// Returns required.
  const auto is_required() const { return m_is_required; }

  /// Sets required.
  control& is_required(bool required)
  {
    m_is_required = required;
    return *this;
  };

  /// Returns line.
  // This is necessary for code wxID_JUMP_TO in ui/listview.cpp.
  const auto line() const { return line_data::line(); }

  /// Sets line number.
  /// Goes to the line if > 0, if -1 goes to end of file
  control& line(int line, std::function<int(int)> f = nullptr)
  {
    line_data::line(line, f);
    return *this;
  };

  /// Returns validator.
  const auto validator() const { return m_validator; }

  /// Sets validator.
  control& validator(wxValidator* validator);

  /// Returns window data.
  const auto& window() const { return m_data; }

  /// Sets window data.
  control& window(const data::window& data)
  {
    m_data = data;
    return *this;
  };

  // Virtual overrides.

  void reset() override;

private:
  data::window m_data;

  bool m_is_required{false};

  int m_find_flags{NUMBER_NOT_SET};

  std::string m_find;

  wxValidator* m_validator{nullptr};
};
}; // namespace wex::data
