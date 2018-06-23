////////////////////////////////////////////////////////////////////////////////
// Name:      listview-data.cpp
// Purpose:   Implementation of wxExListViewData
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/lexer.h>
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
    m_Initialized = r.m_Initialized;
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
  
void wxExListViewData::AddColumns()
{
  m_ListView->AppendColumns({{_("File Name").ToStdString(), wxExColumn::COL_STRING}});

  switch (m_Type)
  {
    case LIST_FIND:
      m_ListView->AppendColumns({
        {_("Line").ToStdString(), wxExColumn::COL_STRING, 250},
        {_("Match").ToStdString(), wxExColumn::COL_STRING},
        {_("Line No").ToStdString()}});
    break;
    case LIST_KEYWORD:
      for (const auto& it : m_Lexer->GetKeywords())
      {
        m_ListView->AppendColumns({{wxExColumn(it)}});
      }

      m_ListView->AppendColumns({{_("Keywords").ToStdString()}});
    break;
    default: break; // to prevent warnings
  }

  m_ListView->AppendColumns({
    {_("Modified").ToStdString(), wxExColumn::COL_DATE},
    {_("In Folder").ToStdString(), wxExColumn::COL_STRING, 175},
    {_("Type").ToStdString(), wxExColumn::COL_STRING},
    {_("Size").ToStdString()}});
}

wxExListViewData& wxExListViewData::Image(wxExImageType type)
{
  m_ImageType = type;
  return *this;
}

bool wxExListViewData::Inject()
{
   bool injected = 
     m_ListView != nullptr && 
     m_ListView->GetItemCount() > 0 && 
     m_Data.Inject(
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

  if (!m_Initialized)
  {
    injected = true;
    m_Initialized = true;
    auto name = TypeDescription();

    switch (m_Type)
    {
      case LIST_FOLDER:
      case LIST_NONE:
        m_ListView->SetSingleStyle(wxLC_LIST);
        break;
      
      case LIST_KEYWORD:
        if (m_Lexer != nullptr)
        {
          name += " " + m_Lexer->GetDisplayLexer();
        }
        // fall through
      default:
        m_ListView->SetSingleStyle(wxLC_REPORT);
        AddColumns();
        break;
    }

    m_ListView->SetName(name);
    m_Data.Window(wxExWindowData().Name(name));
  }

  return injected;
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

const std::string wxExListViewData::TypeDescription() const
{
  wxString value;

  switch (m_Type)
  {
    case LIST_FOLDER: value = _("Folder"); break;
    case LIST_FIND: value = _("Find Results"); break;
    case LIST_HISTORY: value = _("History"); break;
    case LIST_KEYWORD: value = _("Keywords"); break;
    case LIST_FILE: value = _("File"); break;
    case LIST_NONE: value = _("None"); break;
    default: wxFAIL;
  }

  return value.ToStdString();
}
