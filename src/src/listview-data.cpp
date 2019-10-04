////////////////////////////////////////////////////////////////////////////////
// Name:      listview-data.cpp
// Purpose:   Implementation of wex::listview_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
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
  : m_data(data)
  , m_ListView(lv)
{
}

wex::listview_data::listview_data(window_data& data, listview* lv)
  : m_data(control_data().window(data))
  , m_ListView(lv)
{
}

wex::listview_data& wex::listview_data::operator=(const listview_data& r)
{
  if (this != &r)
  {
    m_data = r.m_data;
    m_ImageType = r.m_ImageType;
    m_Initialized = r.m_Initialized;
    m_lexer = r.m_lexer;
    m_menu_flags = r.m_menu_flags;
    m_Type = r.m_Type;

    if (m_ListView != nullptr && r.m_ListView != nullptr)
    {
      m_ListView = r.m_ListView;
    }
  }
  
  return *this;
}
  
void wex::listview_data::add_columns()
{
  m_ListView->append_columns({{_("File Name").ToStdString(), column::STRING}});

  switch (m_Type)
  {
    case FIND:
      m_ListView->append_columns({
        {_("Line").ToStdString(), column::STRING, 250},
        {_("Match").ToStdString(), column::STRING},
        {_("Line No").ToStdString()}});
    break;
    case KEYWORD:
      for (const auto& it : m_lexer->keywords())
      {
        m_ListView->append_columns({{column(it)}});
      }

      m_ListView->append_columns({{_("Keywords").ToStdString()}});
    break;
    default: break; // to prevent warnings
  }

  m_ListView->append_columns({
    {_("Modified").ToStdString(), column::DATE},
    {_("In Folder").ToStdString(), column::STRING, 175},
    {_("Type").ToStdString(), column::STRING},
    {_("Size").ToStdString()}});
}

wex::listview_data& wex::listview_data::image(image_t type)
{
  m_ImageType = type;
  return *this;
}

bool wex::listview_data::inject()
{
   bool injected = 
     m_ListView != nullptr && 
     m_ListView->GetItemCount() > 0 && 
     m_data.inject(
    [&]() {
      const int initial_value = (
        m_ListView->GetFirstSelected() == -1 ? 1: m_ListView->GetFirstSelected() + 1);

      if (initial_value >= 1)
      {
        m_ListView->Select(initial_value - 1, false);
      }
      m_ListView->Select(m_data.line() - 1);
      m_ListView->EnsureVisible(m_data.line() - 1);
      return true;},
    [&]() {return false;},
    [&]() {
      return m_ListView->find_next(m_data.find());},
    [&]() {return false;});

  if (!m_Initialized)
  {
    injected = true;
    m_Initialized = true;
    auto name = type_description();

    switch (m_Type)
    {
      case FOLDER:
      case NONE:
        m_ListView->SetSingleStyle(wxLC_LIST);
        break;
      
      case KEYWORD:
        if (m_lexer != nullptr)
        {
          name += " " + m_lexer->display_lexer();
        }
        // fall through
      default:
        m_ListView->SetSingleStyle(wxLC_REPORT);
        add_columns();
        break;
    }

    m_ListView->SetName(name);
    m_data.window(window_data().name(name));
  }

  return injected;
}
  
wex::listview_data& wex::listview_data::lexer(const wex::lexer* lexer)
{
  m_lexer = lexer;
  return *this;
}

wex::listview_data& wex::listview_data::menu(
  menu_t flags, control_data::action_t action)
{
  m_data.flags<flags.size()>(flags, m_menu_flags, action);
  return *this;
}
  
wex::listview_data& wex::listview_data::type(type_t type)
{
  m_Type = type;
  return *this;
}

const std::string wex::listview_data::type_description() const
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
    default: assert(0);
  }

  return value.ToStdString();
}
