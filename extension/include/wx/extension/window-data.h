////////////////////////////////////////////////////////////////////////////////
// Name:      window-data.h
// Purpose:   Declaration of wex::window_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <wx/window.h>

#define DATA_NUMBER_NOT_SET 0

namespace wex
{
  /// Offers window data to be used by windows or dialogs.
  class window_data
  {
  public:
    /// Default constructor.
    window_data();

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
    window_data& Button(long button);

    /// Returns window id.
    const auto& Id() const {return m_Id;};
    
    /// Sets window id.
    window_data& Id(wxWindowID id);

    /// Returns name.  
    const auto& Name() const {return m_Name;};

    /// Sets window name.
    window_data& Name(const std::string& name);

    /// Returns parent.
    auto Parent() const {return m_Parent;};
    
    /// Sets parent.
    window_data& Parent(wxWindow* parent);

    /// Returns window pos.
    const auto& Pos() const {return m_Pos;};
    
    /// Sets window pos.
    window_data& Pos(const wxPoint& point);

    /// Returns window size.
    const auto& Size() const {return m_Size;};
    
    /// Sets window size.
    window_data& Size(const wxSize& size);

    /// Returns window style.
    const auto& Style() const {return m_Style;};

    /// Sets window style.
    /// The style bits available depend on the context.
    /// Therefore default style is DATA_NUMBER_NOT_SET,
    /// actual style used depend on control
    /// Therefore default style is DATA_NUMBER_NOT_SET,
    /// actual style used depend on active control.
    window_data& Style(long style);

    /// Returns window title.
    const auto& Title() const {return m_Title;};

    /// Sets window title.
    window_data& Title(const std::string& title);
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
};
