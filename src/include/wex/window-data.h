////////////////////////////////////////////////////////////////////////////////
// Name:      window-data.h
// Purpose:   Declaration of wex::window_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <wx/window.h>

namespace wex
{
  const long DATA_NUMBER_NOT_SET = 0;

  /// Offers window data to be used by windows or dialogs.
  class window_data
  {
  public:
    /// Default constructor.
    window_data();

    /// Returns button flags.
    const auto& button() const {return m_Button;};

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
    window_data& button(long button);

    /// Returns window id.
    const auto& id() const {return m_Id;};
    
    /// Sets window id.
    window_data& id(wxWindowID id);

    /// Returns name.  
    const auto& name() const {return m_Name;};

    /// Sets window name.
    window_data& name(const std::string& name);

    /// Returns parent.
    auto parent() const {return m_Parent;};
    
    /// Sets parent.
    window_data& parent(wxWindow* parent);

    /// Returns window pos.
    const auto& pos() const {return m_Pos;};
    
    /// Sets window pos.
    window_data& pos(const wxPoint& point);

    /// Returns window size.
    const auto& size() const {return m_Size;};
    
    /// Sets window size.
    window_data& size(const wxSize& size);

    /// Returns window style.
    const auto& style() const {return m_Style;};

    /// Sets window style.
    /// The style bits available depend on the context.
    /// Therefore default style is DATA_NUMBER_NOT_SET,
    window_data& style(long style);

    /// Returns window title.
    const auto& title() const {return m_Title;};

    /// Sets window title.
    window_data& title(const std::string& title);
  private:  
    wxWindowID m_Id = wxID_ANY;
    wxPoint m_Pos = wxDefaultPosition;
    wxSize m_Size = wxDefaultSize;
    wxWindow* m_Parent = nullptr;
    std::string m_Name, m_Title;
    long m_Button = wxOK | wxCANCEL, m_Style = DATA_NUMBER_NOT_SET;
  };
};
