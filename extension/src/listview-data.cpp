////////////////////////////////////////////////////////////////////////////////
// Name:      listview-data.cpp
// Purpose:   Implementation of wxExListViewData
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/listview-data.h>
#include <wx/extension/listview.h>

wxExListViewData::wxExListViewData(wxExListView* lv)
  : m_ListView(lv)
{
}
  
wxExListViewData::wxExListViewData(wxExListView* lv, const wxExListViewData& r)
  : m_ListView(lv)
{
  *this = r;
}
  
wxExListViewData::wxExListViewData(wxExControlData& data, wxExListView* lv)
  : m_Data(data)
  , m_ListView(lv)
{
}

wxExListViewData::wxExListViewData(wxExWindowData& data, wxExListView* lv)
  : m_Data(wxExControlData().Window(data))
  , m_ListView(lv)
{
}

wxExListViewData& wxExListViewData::operator=(const wxExListViewData& r)
{
  if (this != &r)
  {
    m_Data = r.m_Data;
    m_ImageType = r.m_ImageType;
    m_Lexer = r.m_Lexer;
    m_MenuFlags = r.m_MenuFlags;
    m_Type = r.m_Type;

    if (m_ListView != nullptr && r.m_ListView != nullptr)
    {
      m_ListView = r.m_ListView;
    }
  }
  
  return *this;
}
  
wxExListViewData& wxExListViewData::Image(wxExImageType type)
{
  m_ImageType = type;
  return *this;
}

bool wxExListViewData::Inject() const
{
  return m_ListView != nullptr && m_Data.Inject(
    [&]() {
      const int initial_value = (
        m_ListView->GetFirstSelected() == -1 ? 1: m_ListView->GetFirstSelected() + 1);

      if (initial_value >= 1)
      {
        m_ListView->Select(initial_value - 1, false);
      }
      m_ListView->Select(m_Data.Line() - 1);
      m_ListView->EnsureVisible(m_Data.Line() - 1);
      return true;},
    [&]() {return false;},
    [&]() {
      return m_ListView->FindNext(m_Data.Find());},
    [&]() {return false;});
}
  
wxExListViewData& wxExListViewData::Lexer(const wxExLexer* lexer)
{
  m_Lexer = lexer;
  return *this;
}

wxExListViewData& wxExListViewData::Menu(long flags, wxExDataAction action)
{
  m_Data.Flags<long>(flags, m_MenuFlags, action);
  return *this;
}
  
wxExListViewData& wxExListViewData::Type(wxExListType type)
{
  m_Type = type;
  return *this;
}
