////////////////////////////////////////////////////////////////////////////////
// Name:      control-data.h
// Purpose:   Declaration of wex::control_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <string>
#include <wx/validate.h>
#include <wex/ex-command.h>
#include <wex/window-data.h>

namespace wex
{
  /// Offers user data to be used by ctrl classes (as listctrl, styledtextctrl).
  /// First you can set the data using Col, Line, Find etc.,
  /// then call Inject to perform the action.
  /// You can set several items, Inject prioritizes the actions.
  class control_data
  {
  public:
    /// Determine how flags value are set.
    enum action
    {
      SET, /// set value
      OR,  /// add this flag
      INV, /// remove this flag
      XOR, /// xor this flag
    };

    /// Returns column.
    const auto& Col() const {return m_Col;};
    
    /// Sets column.
    /// Goes to column if col_number > 0
    control_data& Col(int col);
    
    /// Returns command.
    const auto& Command() const {return m_Command;};
    
    /// Sets command.
    /// This is a vi command to execute.
    control_data& Command(const std::string& command);

    /// Returns find.
    const auto& Find() const {return m_Find;};
    
    /// Sets find.
    /// If not empty selects the text on that line (if line was specified)
    /// or finds text from begin (if line was 0) or end (line was -1).
    control_data& Find(
      /// text to find
      const std::string& text,
      /// find flags to be used
      int find_flags = 0);

    /// Returns find flags.
    const auto FindFlags() const {return m_FindFlags;};
    
    /// Sets specified flags.
    /// This is used by the other data classes as generic 
    /// method to operate on flags.
    template<typename T>
    control_data& Flags(
      T flags, T& result, 
      action action = SET) {
      switch (action)
      {
        case INV: result = static_cast<T>(result & ~flags); break;
        case OR:  result = static_cast<T>(result | flags); break;
        case SET: result = flags; break;
        case XOR: result = static_cast<T>(result ^ flags); break;
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
    control_data& Line(int line, std::function<int(int)> valid = nullptr);

    /// Resets members to default state.
    void Reset();
    
    /// Returns required.
    const auto Required() const {return m_Required;};
    
    /// Sets required.
    control_data& Required(bool required) {m_Required = required; return *this;};

    /// Returns validator.
    const auto Validator() const {return m_Validator;};

    /// Sets validator.
    control_data& Validator(wxValidator* validator);

    /// Returns window data.
    const auto& Window() const {return m_Data;};

    /// Sets window data.
    control_data& Window(window_data& data) {m_Data = data; return *this;};
  private:  
    window_data m_Data;

    bool m_Required {false};
    
    int 
      m_Col {DATA_NUMBER_NOT_SET}, 
      m_FindFlags {DATA_NUMBER_NOT_SET}, 
      m_Line {DATA_NUMBER_NOT_SET};
    
    std::string m_Find {std::string()};
    
    ex_command m_Command {std::string()};
    wxValidator* m_Validator {nullptr};
  };
};
