////////////////////////////////////////////////////////////////////////////////
// Name:      configitem.cpp
// Purpose:   Implementation of wxExConfigItem class
// Author:    Anton van Wezenbeek
// Created:   2009-11-10
// RCS-ID:    $Id$
// Copyright: (c) 2009 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/configitem.h>

#if wxUSE_GUI
wxExConfigItem::wxExConfigItem()
  : m_Name("spacer")
  , m_Page(wxEmptyString)
  , m_Type(CONFIG_SPACER) 
{
}

wxExConfigItem::wxExConfigItem(
  const wxString& name,
  int min,
  int max,
  const wxString& page)
  : m_IsRequired(false)
  , m_Min(min)
  , m_Max(max)
  , m_MaxItems(0)
  , m_Name(name)
  , m_Page(page)
  , m_Style(0)
  , m_Type(CONFIG_SPINCTRL)
{
}

wxExConfigItem::wxExConfigItem(
  const wxString& name,
  double min,
  double max,
  double inc,
  const wxString& page)
  : m_IsRequired(false)
  , m_MaxItems(0)
  , m_MinDouble(min)
  , m_MaxDouble(max)
  , m_Inc(inc)
  , m_Name(name)
  , m_Page(page)
  , m_Style(0)
  , m_Type(CONFIG_SPINCTRL_DOUBLE)
{
}

wxExConfigItem::wxExConfigItem(
  const wxString& name,
  const wxString& page,
  long style,
  bool is_required)
  : m_IsRequired(is_required)
  , m_Min(0)
  , m_Max(0)
  , m_MaxItems(0)
  , m_Name(name)
  , m_Page(page)
  , m_Style(style)
  , m_Type(CONFIG_STRING)
{
}

wxExConfigItem::wxExConfigItem(
  const wxString& name,
  const std::map<long, const wxString> & choices,
  bool use_radiobox,
  const wxString& page)
  : m_IsRequired(false)
  , m_Min(0)
  , m_Max(0)
  , m_MaxItems(0)
  , m_Name(name)
  , m_Page(page)
  , m_Style(0)
  , m_Type(use_radiobox ? CONFIG_RADIOBOX: CONFIG_CHECKLISTBOX)
  , m_Choices(choices)
{
}

wxExConfigItem::wxExConfigItem(
  const std::set<wxString> & choices,
  const wxString& page)
  : m_IsRequired(false)
  , m_Min(0)
  , m_Max(0)
  , m_MaxItems(0)
  , m_Name("checklistbox_noname")
  , m_Page(page)
  , m_Style(0)
  , m_Type(CONFIG_CHECKLISTBOX_NONAME)
  , m_ChoicesBool(choices) 
{
}

wxExConfigItem::wxExConfigItem(
  const wxString& name,
  int type,
  const wxString& page,
  bool is_required,
  int max_items)
  : m_IsRequired(is_required)
  , m_Min(0)
  , m_Max(0)
  , m_MaxItems(max_items)
  , m_Name(name)
  , m_Page(page)
  , m_Style(0)
  , m_Type(type) 
{
}
#endif // wxUSE_GUI
