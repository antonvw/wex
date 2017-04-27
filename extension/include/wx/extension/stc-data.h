////////////////////////////////////////////////////////////////////////////////
// Name:      stc-data.h
// Purpose:   Declaration of wxExSTCData
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <wx/dlimpexp.h>
#include <wx/window.h>
#include <wx/extension/stc-enums.h>

const int DATA_INT_NOT_SET = 0;

/// Determine how flags value are set.
enum wxExDataAction
{
  DATA_SET, /// set value
  DATA_OR,  /// add this flag
  DATA_INV, /// remove this flag
};

class wxExSTC;

/// Offers user data to be used by STC. 
/// First you can set the data using Col, Line, Find etc.,
/// then call Inject to perform the action.
/// When you set several items, Inject prioritizes
/// the actions.
class WXDLLIMPEXP_BASE wxExSTCData
{
public:
  /// Default constructor.
  wxExSTCData(wxExSTC* stc = nullptr);

  /// Copy constructor.
  wxExSTCData(wxExSTC* stc, const wxExSTCData& r);

  /// Assignment operator.
  wxExSTCData& operator=(const wxExSTCData& r);
  
  /// Returns column.
  const auto& Col() const {return m_Col;};
  
  /// Sets column.
  /// Goes to column if col_number > 0
  wxExSTCData& Col(int col);
  
  /// Returns command.
  const auto& Command() const {return m_Command;};
  
  /// Sets command.
  /// This is a vi command to execute.
  wxExSTCData& Command(const std::string& command);
  
  /// Returns find.
  const auto& Find() const {return m_Find;};
  
  /// Sets find.
  /// If not empty selects the text on that line (if line was specified)
  /// or finds text from begin (if line was 0) or end (line was -1).
  wxExSTCData& Find(const std::string& text);
  
  /// Returns window flags.
  const auto& Flags() const {return m_WinFlags;};
  
  /// Set window flags.
  wxExSTCData& Flags(wxExSTCWindowFlags flags, wxExDataAction action = DATA_SET);
  
  /// If there is a stc component, uses
  /// current data to goto the requested position.
  /// Returns false if no suitable data was present or stc is nullpr.
  bool Inject() const;

  /// Returns line number.
  const auto Line() const {return m_Line;};
  
  /// Sets line number.
  /// Goes to the line if > 0, if -1 goes to end of file
  wxExSTCData& Line(int line);
  
  /// Returns menu flags.
  const auto& Menu() const {return m_MenuFlags;};

  /// Sets menu flags.
  wxExSTCData& Menu(wxExSTCMenuFlags flags, wxExDataAction action = DATA_SET);
private:  
  template<typename T>
  wxExSTCData& Flags(T flags, T& result, wxExDataAction action = DATA_SET) {
    switch (action)
    {
      case DATA_INV: result = 
        static_cast<T>(result & ~flags); 
        break;
      case DATA_OR: result = 
        static_cast<T>(result | flags); 
        break;
      case DATA_SET: result = flags;
        break;
    }
    return *this;};
  
  void InjectCol() const;
  void InjectFind() const;
  void InjectLine() const;

  int ValidLine(int line) const;
  
  wxExSTC* m_STC = nullptr;
  
  int m_Col = DATA_INT_NOT_SET;
  int m_Line = DATA_INT_NOT_SET;
  
  std::string m_Command = std::string();
  std::string m_Find = std::string();

  wxExSTCMenuFlags m_MenuFlags = static_cast<wxExSTCMenuFlags>(
    STC_MENU_CONTEXT | STC_MENU_OPEN_LINK | STC_MENU_VCS);
  wxExSTCWindowFlags m_WinFlags = STC_WIN_DEFAULT;
};

class WXDLLIMPEXP_BASE wxExWindowData
{
public:
  /// Default constructor.
  wxExWindowData(
    const wxPoint pos = wxDefaultPosition,
    const wxSize size = wxDefaultSize)
    : m_Pos(pos)
    , m_Size(size) {;};
  
  /// Returns window id.
  const auto& Id() const {return m_Id;};
  
  /// Sets window id.
  wxExWindowData& Id(wxWindowID id);
  
  /// Returns window pos.
  const auto& Pos() const {return m_Pos;};
  
  /// Returns window size.
  const auto& Size() const {return m_Size;};
  
  /// Returns window syle.
  const auto& Style() const {return m_Style;};

  /// Sets window style.
  wxExWindowData& Style(long style);
private:  
  wxWindowID m_Id = wxID_ANY;
  const wxPoint m_Pos;
  const wxSize m_Size;
  long m_Style = 0;
};
