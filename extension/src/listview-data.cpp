////////////////////////////////////////////////////////////////////////////////
// Name:      listview-data.cpp
// Purpose:   Implementation of wex::listview_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/lexer.h>
#include <wex/listview-data.h>
#include <wex/listview.h>

wex::listview_data::listview_data(listview* lv)
  : m_ListView(lv)
{
}
  
wex::listview_data::listview_data(listview* lv, const listview_data& r)
  : m_ListView(lv)
{
  *this = r;
}
  
wex::listview_data::listview_data(control_data& data, listview* lv)
  : m_Data(data)
  , m_ListView(lv)
{
}

wex::listview_data::listview_data(window_data& data, listview* lv)
  : m_Data(control_data().Window(data))
  , m_ListView(lv)
{
}

wex::listview_data& wex::listview_data::operator=(const listview_data& r)
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
  
void wex::listview_data::AddColumns()
{
  m_ListView->AppendColumns({{_("File Name").ToStdString(), column::STRING}});

  switch (m_Type)
  {
    case FIND:
      m_ListView->AppendColumns({
        {_("Line").ToStdString(), column::STRING, 250},
        {_("Match").ToStdString(), column::STRING},
        {_("Line No").ToStdString()}});
    break;
    case KEYWORD:
      for (const auto& it : m_Lexer->GetKeywords())
      {
        m_ListView->AppendColumns({{column(it)}});
      }

      m_ListView->AppendColumns({{_("Keywords").ToStdString()}});
    break;
    default: break; // to prevent warnings
  }

  m_ListView->AppendColumns({
    {_("Modified").ToStdString(), column::DATE},
    {_("In Folder").ToStdString(), column::STRING, 175},
    {_("Type").ToStdString(), column::STRING},
    {_("Size").ToStdString()}});
}

wex::listview_data& wex::listview_data::Image(image_type type)
{
  m_ImageType = type;
  return *this;
}

bool wex::listview_data::Inject()
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
      case FOLDER:
      case NONE:
        m_ListView->SetSingleStyle(wxLC_LIST);
        break;
      
      case KEYWORD:
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
    m_Data.Window(window_data().Name(name));
  }

  return injected;
}
  
wex::listview_data& wex::listview_data::Lexer(const lexer* lexer)
{
  m_Lexer = lexer;
  return *this;
}

wex::listview_data& wex::listview_data::Menu(
  long flags, control_data::action action)
{
  m_Data.Flags<long>(flags, m_MenuFlags, action);
  return *this;
}
  
wex::listview_data& wex::listview_data::Type(type type)
{
  m_Type = type;
  return *this;
}

const std::string wex::listview_data::TypeDescription() const
{
  wxString value;

  switch (m_Type)
  {
    case FOLDER: value = _("Folder"); break;
    case FIND: value = _("Find Results"); break;
    case HISTORY: value = _("History"); break;
    case KEYWORD: value = _("Keywords"); break;
    case FILE: value = _("File"); break;
    case NONE: value = _("None"); break;
    default: wxFAIL;
  }

  return value.ToStdString();
}
