////////////////////////////////////////////////////////////////////////////////
// Name:      data/toolbar.h
// Purpose:   Declaration of wex::data::toolbar_item
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <wx/bitmap.h>
#include <wx/defs.h>

namespace wex::data
{
  /// This class offers data for a toolbar item.
  class toolbar_item
  {
  public:
    /// Constructor, specify tool id.
    toolbar_item(int id)
      : m_id(id)
    {
      ;
    };

    /// Returns bitmap.
    const wxBitmap& bitmap() const { return m_bitmap; };

    /// Sets bitmap.
    toolbar_item& bitmap(const wxBitmap& rhs);

    /// Returns id.
    int id() const { return m_id; };

    /// Returns kind.
    wxItemKind kind() const { return m_kind; };

    /// Sets kind.
    toolbar_item& kind(wxItemKind rhs);

    /// Returns label.
    const std::string& label() const { return m_label; };

    /// Sets label.
    toolbar_item& label(const std::string& rhs);

    /// Returns help.
    const std::string& help() const { return m_short_help; };

    /// Sets help.
    toolbar_item& help(const std::string& rhs);

  private:
    wxBitmap    m_bitmap{wxNullBitmap};
    const int   m_id;
    wxItemKind  m_kind{wxITEM_NORMAL};
    std::string m_short_help, m_label;
  };
}; // namespace wex::data
