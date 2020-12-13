////////////////////////////////////////////////////////////////////////////////
// Name:      control-data.h
// Purpose:   Declaration of wex::data::control
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <functional>
#include <string>
#include <wex/window-data.h>
#include <wx/validate.h>

namespace wex::data
{
  /// Offers user data to be used by ctrl classes (as listctrl, styledtextctrl).
  /// First you can set the data using Col, Line, Find etc.,
  /// then call inject to perform the action.
  /// You can set several items, inject prioritizes the actions.
  class control
  {
  public:
    /// Determine how flags value are set.
    enum action_t
    {
      INV, /// sets value and flips result
      NOT, /// remove this flag
      OR,  /// add this flag
      SET, /// set value
      XOR, /// xor this flag
    };

    /// Returns column.
    const auto& col() const { return m_col; };

    /// Sets column.
    /// Goes to column if col_number > 0
    control& col(int col);

    /// Returns command.
    const auto& command() const { return m_command; };

    /// Sets command.
    /// This is a vi command to execute.
    control& command(const std::string& command);

    /// Returns find.
    const auto& find() const { return m_find; };

    /// Sets find.
    /// If not empty selects the text on that line (if line was specified)
    /// or finds text from begin (if line was 0) or end (line was -1).
    control& find(
      /// text to find
      const std::string& text,
      /// find flags to be used
      int find_flags = 0);

    /// Returns find flags.
    const auto find_flags() const { return m_find_flags; };

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
      std::function<bool(void)> line = nullptr,
      /// callback to inject column number
      std::function<bool(void)> col = nullptr,
      /// callback to inject find text
      std::function<bool(void)> find = nullptr,
      /// callback to inject vi command
      std::function<bool(void)> command = nullptr) const;

    /// Returns required.
    const auto is_required() const { return m_is_required; };

    /// Sets required.
    control& is_required(bool required)
    {
      m_is_required = required;
      return *this;
    };

    /// Returns line number.
    const auto line() const { return m_line; };

    /// Sets line number.
    /// Goes to the line if > 0, if -1 goes to end of file
    control& line(int line, std::function<int(int)> valid = nullptr);

    /// Resets members to default state.
    void reset();

    /// Returns validator.
    const auto validator() const { return m_validator; };

    /// Sets validator.
    control& validator(wxValidator* validator);

    /// Returns window data.
    const auto& window() const { return m_data; };

    /// Sets window data.
    control& window(const data::window& data)
    {
      m_data = data;
      return *this;
    };

  private:
    data::window m_data;

    bool m_is_required{false};

    int m_col{NUMBER_NOT_SET}, m_find_flags{NUMBER_NOT_SET},
      m_line{NUMBER_NOT_SET};

    std::string m_find, m_command;

    wxValidator* m_validator{nullptr};
  };
}; // namespace wex::data
