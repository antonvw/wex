////////////////////////////////////////////////////////////////////////////////
// Name:      window-data.h
// Purpose:   Declaration of wxExWindowData
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <wx/dlimpexp.h>
#include <wx/window.h>

#define DATA_NUMBER_NOT_SET 0

/// Offers window data to be used by windows or dialogs.
class WXDLLIMPEXP_BASE wxExWindowData
{
public:
  /// Default constructor.
  wxExWindowData();

  /// Returns button flags.
  const auto& Button() const {return m_Button;};

  /// Sets buttons flags.
  /// This is a bit list of the following flags:
  /// - wxOK 
  /// - wxYES 
  /// - wxAPPLY
  /// - wxSAVE 
  /// - wxCLOSE
  /// - wxNO
  /// - wxCANCEL
  /// - wxHELP
  /// - wxNO_DEFAULT
  /// The default buttons are wxOK and wxCANCEL
  wxExWindowData& Button(long button);

  /// Returns window id.
  const auto& Id() const {return m_Id;};
  
  /// Sets window id.
  wxExWindowData& Id(wxWindowID id);

  /// Returns name.  
  const auto& Name() const {return m_Name;};

  /// Sets window name.
  wxExWindowData& Name(const std::string& name);

  /// Returns parent.
  auto Parent() const {return m_Parent;};
  
  /// Sets parent.
  wxExWindowData& Parent(wxWindow* parent);

  /// Returns window pos.
  const auto& Pos() const {return m_Pos;};
  
  /// Sets window pos.
  wxExWindowData& Pos(const wxPoint& point);

  /// Returns window size.
  const auto& Size() const {return m_Size;};
  
  /// Sets window size.
  wxExWindowData& Size(const wxSize& size);

  /// Returns window style.
  const auto& Style() const {return m_Style;};

  /// Sets window style.
  /// The style bits available depend on the context.
  /// Therefore default style is DATA_NUMBER_NOT_SET,
  /// actual style used depend on control
  /// Therefore default style is DATA_NUMBER_NOT_SET,
  /// actual style used depend on active control.
  wxExWindowData& Style(long style);

  /// Returns window title.
  const auto& Title() const {return m_Title;};

  /// Sets window title.
  wxExWindowData& Title(const std::string& title);
private:  
  wxWindowID m_Id = wxID_ANY;
  wxPoint m_Pos = wxDefaultPosition;
  wxSize m_Size = wxDefaultSize;
  wxWindow* m_Parent = nullptr;
  std::string m_Name;
  std::string m_Title;
  long m_Button = wxOK | wxCANCEL;
  long m_Style = DATA_NUMBER_NOT_SET;
};
