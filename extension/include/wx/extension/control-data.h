////////////////////////////////////////////////////////////////////////////////
// Name:      control-data.h
// Purpose:   Declaration of wxExControlData
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <string>
#include <wx/dlimpexp.h>
#include <wx/validate.h>
#include <wx/extension/window-data.h>

/// Determine how flags value are set.
enum wxExDataAction
{
  DATA_SET, /// set value
  DATA_OR,  /// add this flag
  DATA_INV, /// remove this flag
  DATA_XOR, /// xor this flag
};

/// Offers user data to be used by ctrl classes (as listctrl, styledtextctrl).
/// First you can set the data using Col, Line, Find etc.,
/// then call Inject to perform the action.
/// You can set several items, Inject prioritizes the actions.
class WXDLLIMPEXP_BASE wxExControlData
{
public:
  /// Returns column.
  const auto& Col() const {return m_Col;};
  
  /// Sets column.
  /// Goes to column if col_number > 0
  wxExControlData& Col(int col);
  
  /// Returns command.
  const auto& Command() const {return m_Command;};
  
  /// Sets command.
  /// This is a vi command to execute.
  wxExControlData& Command(const std::string& command);

  /// Returns find.
  const auto& Find() const {return m_Find;};
  
  /// Sets find.
  /// If not empty selects the text on that line (if line was specified)
  /// or finds text from begin (if line was 0) or end (line was -1).
  wxExControlData& Find(const std::string& text);

  /// Sets specified flags.  
  template<typename T>
  wxExControlData& Flags(T flags, T& result, wxExDataAction action = DATA_SET) {
    switch (action)
    {
      case DATA_INV: result = static_cast<T>(result & ~flags); break;
      case DATA_OR:  result = static_cast<T>(result | flags); break;
      case DATA_SET: result = flags; break;
      case DATA_XOR: result = static_cast<T>(result ^ flags); break;
    }
    return *this;};

  /// Injects data.
  /// If there is a callback specified, injects current data in it:
  /// - if line available: goto line
  /// - if col available: goto col
  /// - if text not empty: finds text
  /// - if vi command not empty: executes vi command
  /// - if flags are set: sets flag on Control
  /// Returns true if data coulld be injected into the control.
  bool Inject(
    /// callback to inject line number
    std::function<bool(void)> line = nullptr,
    /// callback to inject column number
    std::function<bool(void)> col = nullptr,
    /// callback to inject find text
    std::function<bool(void)> find = nullptr,
    /// callback to inject vi command
    std::function<bool(void)> command = nullptr) const;

  /// Returns line number.
  const auto Line() const {return m_Line;};
  
  /// Sets line number.
  /// Goes to the line if > 0, if -1 goes to end of file
  wxExControlData& Line(int line, std::function<int(int)> valid = nullptr);

  /// Resets members to default state.
  void Reset();
  
  /// Returns required.
  const auto Required() const {return m_Required;};
  
  /// Sets required.
  wxExControlData& Required(bool required) {m_Required = required; return *this;};

  /// Returns validator.
  const auto Validator() const {return m_Validator;};

  /// Sets validator.
  wxExControlData& Validator(wxValidator* validator);

  /// Returns window data.
  const auto& Window() const {return m_Data;};

  /// Sets window data.
  wxExControlData& Window(wxExWindowData& data) {m_Data = data; return *this;};
private:  
  wxExWindowData m_Data;

  bool m_Required = false;
  
  int m_Col = DATA_NUMBER_NOT_SET;
  int m_Line = DATA_NUMBER_NOT_SET;
  
  std::string m_Command = std::string();
  std::string m_Find = std::string();

  wxValidator* m_Validator = nullptr;
};
