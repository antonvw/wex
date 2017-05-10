////////////////////////////////////////////////////////////////////////////////
// Name:      listview-data.h
// Purpose:   Declaration of wxExListViewData
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/dlimpexp.h>
#include <wx/extension/control-data.h>
#include <wx/extension/listview-enums.h>

class wxExLexer;
class wxExListView;

/// Offers user data to be used by wxExListView. 
class WXDLLIMPEXP_BASE wxExListViewData
{
public:
  /// Default constructor.
  wxExListViewData(wxExListView* lv = nullptr);

  /// Copy constructor.
  wxExListViewData(wxExListView* lv, const wxExListViewData& r);

  /// Constructor from control data.
  wxExListViewData(wxExControlData& data, wxExListView* lv = nullptr);

  /// Constructor from window data.
  wxExListViewData(wxExWindowData& data, wxExListView* lv = nullptr);

  /// Assignment operator.
  wxExListViewData& operator=(const wxExListViewData& r);

  /// Returns control data.
  const auto& Control() const {return m_Data;};

  /// Sets control data.
  wxExListViewData& Control(wxExControlData& data) {m_Data = data; return *this;};
  
  /// Returns image type.
  const auto& Image() const {return m_ImageType;};

  /// Sets image type.
  wxExListViewData& Image(wxExImageType type);

  /// Injects data.  
  bool Inject() const;

  /// Returns lexer.
  const auto& Lexer() const {return m_Lexer;};

  /// Sets lexer.
  wxExListViewData& Lexer(const wxExLexer* lexer);

  /// Returns menu flags.
  const auto& Menu() const {return m_MenuFlags;};

  /// Sets menu flags.
  wxExListViewData& Menu(long flags, wxExDataAction action = DATA_SET);

  /// Returns type.
  const auto& Type() const {return m_Type;};
  
  /// Set type.
  wxExListViewData& Type(wxExListType type);

  /// Returns window data.
  const auto& Window() const {return m_Data.Window();};

  /// Sets window data.
  wxExListViewData& Window(wxExWindowData& data) {m_Data.Window(data); return *this;};
private:  
  wxExControlData m_Data;

  long m_MenuFlags = LIST_MENU_DEFAULT;

  const wxExLexer* m_Lexer = nullptr;
  wxExListView* m_ListView = nullptr;

  wxExImageType m_ImageType = IMAGE_ART;
  wxExListType m_Type = LIST_NONE;
};
