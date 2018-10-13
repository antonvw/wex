////////////////////////////////////////////////////////////////////////////////
// Name:      listview-data.h
// Purpose:   Declaration of wex::listview_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/control-data.h>
#include <wx/extension/listview-enums.h>

namespace wex
{
  class lexer;
  class listview;

  /// Offers user data to be used by listview. 
  class listview_data
  {
  public:
    /// Default constructor.
    listview_data(listview* lv = nullptr);

    /// Copy constructor.
    listview_data(listview* lv, const listview_data& r);

    /// Constructor from control data.
    listview_data(control_data& data, listview* lv = nullptr);

    /// Constructor from window data.
    listview_data(window_data& data, listview* lv = nullptr);

    /// Assignment operator.
    listview_data& operator=(const listview_data& r);

    /// Returns control data.
    const auto& Control() const {return m_Data;};

    /// Sets control data.
    listview_data& Control(control_data& data) {m_Data = data; return *this;};
    
    /// Returns image type.
    const auto& Image() const {return m_ImageType;};

    /// Sets image type.
    listview_data& Image(image_type type);

    /// Injects data.  
    bool Inject();

    /// Returns lexer.
    const auto& Lexer() const {return m_Lexer;};

    /// Sets lexer.
    listview_data& Lexer(const lexer* lexer);

    /// Returns menu flags.
    const auto& Menu() const {return m_MenuFlags;};

    /// Sets menu flags.
    listview_data& Menu(long flags, data_action action = DATA_SET);

    /// Returns type.
    const auto& Type() const {return m_Type;};
    
    /// Sets listview_type.
    listview_data& Type(listview_type type);

    /// Returns the list type as a string.
    const std::string TypeDescription() const;

    /// Returns window data.
    const auto& Window() const {return m_Data.Window();};

    /// Sets window data.
    listview_data& Window(window_data& data) {m_Data.Window(data); return *this;};
  private:  
    void AddColumns();

    control_data m_Data;

    long m_MenuFlags = LISTVIEW_MENU_DEFAULT;

    const lexer* m_Lexer = nullptr;
    listview* m_ListView = nullptr;

    image_type m_ImageType = IMAGE_ART;
    listview_type m_Type = LISTVIEW_NONE;

    bool m_Initialized = false;
  };
};
