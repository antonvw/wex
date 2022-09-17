////////////////////////////////////////////////////////////////////////////////
// Name:      data/listview.cpp
// Purpose:   Implementation of wex::data::listview
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/listview.h>
#include <wex/factory/listview.h>
#include <wex/syntax/lexer.h>

wex::data::listview::listview(factory::listview* lv)
  : m_listview(lv)
{
}

wex::data::listview::listview(factory::listview* lv, const data::listview& r)
  : m_listview(lv)
{
  *this = r;
}

wex::data::listview::listview(data::control& data, factory::listview* lv)
  : m_data(data)
  , m_listview(lv)
{
}

wex::data::listview::listview(data::window& data, factory::listview* lv)
  : m_data(data::control().window(data))
  , m_listview(lv)
{
}

wex::data::listview& wex::data::listview::operator=(const data::listview& r)
{
  if (this != &r)
  {
    m_data        = r.m_data;
    m_image_type  = r.m_image_type;
    m_initialized = r.m_initialized;
    m_lexer       = r.m_lexer;
    m_menu_flags  = r.m_menu_flags;
    m_type        = r.m_type;

    if (m_listview != nullptr && r.m_listview != nullptr)
    {
      m_listview = r.m_listview;
    }
  }

  return *this;
}

void wex::data::listview::add_columns()
{
  m_listview->append_columns({{_("File Name"), column::STRING}});

  switch (m_type)
  {
    case FIND:
      m_listview->append_columns(
        {{_("Line"), column::STRING, 250},
         {_("Match"), column::STRING},
         {_("Line No")}});
      break;
    default:
      break; // to prevent warnings
  }

  m_listview->append_columns(
    {{_("Modified"), column::DATE},
     {_("In Folder"), column::STRING, 175},
     {_("Type"), column::STRING},
     {_("Size")}});
}

wex::data::listview& wex::data::listview::image(image_t type)
{
  m_image_type = type;
  return *this;
}

bool wex::data::listview::inject()
{
  bool injected = m_listview != nullptr && m_listview->GetItemCount() > 0 &&
                  m_data.inject(
                    [&]()
                    {
                      const int initial_value =
                        (m_listview->GetFirstSelected() == -1 ?
                           1 :
                           m_listview->GetFirstSelected() + 1);

                      if (initial_value >= 1)
                      {
                        m_listview->Select(initial_value - 1, false);
                      }
                      m_listview->Select(m_data.line() - 1);
                      m_listview->EnsureVisible(m_data.line() - 1);
                      return true;
                    },
                    [&]()
                    {
                      return false;
                    },
                    [&]()
                    {
                      return m_listview->find_next(m_data.find());
                    },
                    [&]()
                    {
                      return false;
                    });

  if (!m_initialized)
  {
    injected      = true;
    m_initialized = true;
    auto name     = type_description();

    switch (m_type)
    {
      case FOLDER:
      case NONE:
        m_listview->SetSingleStyle(wxLC_LIST);
        break;

      case TSV:
        m_listview->SetSingleStyle(wxLC_REPORT);
        break;

      default:
        m_listview->SetSingleStyle(wxLC_REPORT);
        add_columns();
        break;
    }

    m_listview->SetName(name);
    m_data.window(data::window().name(name));
  }

  return injected;
}

wex::data::listview& wex::data::listview::lexer(const wex::lexer* lexer)
{
  m_lexer = lexer;
  return *this;
}

wex::data::listview&
wex::data::listview::menu(menu_t flags, data::control::action_t action)
{
  m_data.flags<flags.size()>(flags, m_menu_flags, action);
  return *this;
}

wex::data::listview& wex::data::listview::type(type_t type)
{
  m_type = type;
  return *this;
}

const std::string wex::data::listview::type_description() const
{
  std::string value;

  switch (m_type)
  {
    case FOLDER:
      value = _("Folder");
      break;
    case FIND:
      value = _("Find Results");
      break;
    case HISTORY:
      value = _("History");
      break;
    case FILE:
      value = _("File");
      break;
    case TSV:
      value = "tsv";
      break;
    case NONE:
      value = _("None");
      break;
    default:
      assert(0);
  }

  return value;
}
