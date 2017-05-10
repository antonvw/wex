////////////////////////////////////////////////////////////////////////////////
// Name:      stc-data.h
// Purpose:   Declaration of wxExSTCData
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/dlimpexp.h>
#include <wx/extension/control-data.h>
#include <wx/extension/stc-enums.h>

class wxExSTC;

/// Offers user data to be used by wxExSTC. 
class WXDLLIMPEXP_BASE wxExSTCData
{
public:
  /// Default constructor.
  wxExSTCData(wxExSTC* stc = nullptr);

  /// Constructor from control data.
  wxExSTCData(wxExControlData& data, wxExSTC* stc = nullptr);

  /// Constructor from window data.
  wxExSTCData(wxExWindowData& data, wxExSTC* stc = nullptr);

  /// Copy constructor.
  wxExSTCData(wxExSTC* stc, const wxExSTCData& r);

  /// Assignment operator.
  wxExSTCData& operator=(const wxExSTCData& r);
  
  /// Returns control data.
  const auto& Control() const {return m_Data;};

  /// Sets control data.
  wxExSTCData& Control(wxExControlData& data) {m_Data = data; return *this;};

  /// Returns window flags.
  const auto& Flags() const {return m_WinFlags;};
  
  /// Set window flags.
  wxExSTCData& Flags(wxExSTCWindowFlags flags, wxExDataAction action = DATA_SET);

  /// Injects data.  
  bool Inject() const;

  /// Returns menu flags.
  const auto& Menu() const {return m_MenuFlags;};

  /// Sets menu flags.
  wxExSTCData& Menu(wxExSTCMenuFlags flags, wxExDataAction action = DATA_SET);

  /// Returns window data.
  const auto& Window() const {return m_Data.Window();};

  /// Sets window data.
  wxExSTCData& Window(wxExWindowData& data) {m_Data.Window(data); return *this;};
private:  
  wxExSTC* m_STC = nullptr;

  wxExControlData m_Data;

  wxExSTCMenuFlags m_MenuFlags = static_cast<wxExSTCMenuFlags>(
    STC_MENU_CONTEXT | STC_MENU_OPEN_LINK | STC_MENU_VCS);
  wxExSTCWindowFlags m_WinFlags = STC_WIN_DEFAULT;
};
