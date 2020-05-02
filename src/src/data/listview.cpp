////////////////////////////////////////////////////////////////////////////////
// Name:      data/listview.cpp
// Purpose:   Implementation of wex::listview_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/lexer.h>
#include <wex/listview-data.h>
#include <wex/listview.h>

wex::listview_data::listview_data(listview* lv)
  : m_listview(lv)
{
}

wex::listview_data::listview_data(listview* lv, const listview_data& r)
  : m_listview(lv)
{
  *this = r;
}

wex::listview_data::listview_data(control_data& data, listview* lv)
  : m_data(data)
  , m_listview(lv)
{
}

wex::listview_data::listview_data(window_data& data, listview* lv)
  : m_data(control_data().window(data))
  , m_listview(lv)
{
}

wex::listview_data& wex::listview_data::operator=(const listview_data& r)
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

void wex::listview_data::add_columns()
{
  m_listview->append_columns({{_("File Name"), column::STRING}});

  switch (m_type)
  {
    case FIND:
      m_listview->append_columns({{_("Line"), column::STRING, 250},
                                  {_("Match"), column::STRING},
                                  {_("Line No")}});
      break;
    case KEYWORD:
      for (const auto& it : m_lexer->keywords())
      {
        m_listview->append_columns({{column(it)}});
      }

      m_listview->append_columns({{_("Keywords")}});
      break;
    default:
      break; // to prevent warnings
  }

  m_listview->append_columns({{_("Modified"), column::DATE},
                              {_("In Folder"), column::STRING, 175},
                              {_("Type"), column::STRING},
                              {_("Size")}});
}

wex::listview_data& wex::listview_data::image(image_t type)
{
  m_image_type = type;
  return *this;
}

bool wex::listview_data::inject()
{
  bool injected = m_listview != nullptr && m_listview->GetItemCount() > 0 &&
                  m_data.inject(
                    [&]() {
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
                    [&]() {
                      return false;
                    },
                    [&]() {
                      return m_listview->find_next(m_data.find());
                    },
                    [&]() {
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
      case TSV:
        m_listview->SetSingleStyle(wxLC_LIST);
        break;

      case KEYWORD:
        if (m_lexer != nullptr)
        {
          name += " " + m_lexer->display_lexer();
        }
        // fall through
      default:
        m_listview->SetSingleStyle(wxLC_REPORT);
        add_columns();
        break;
    }

    m_listview->SetName(name);
    m_data.window(window_data().name(name));
  }

  return injected;
}

wex::listview_data& wex::listview_data::lexer(const wex::lexer* lexer)
{
  m_lexer = lexer;
  return *this;
}

wex::listview_data&
wex::listview_data::menu(menu_t flags, control_data::action_t action)
{
  m_data.flags<flags.size()>(flags, m_menu_flags, action);
  return *this;
}

wex::listview_data& wex::listview_data::type(type_t type)
{
  m_type = type;
  return *this;
}

const std::string wex::listview_data::type_description() const
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
    case KEYWORD:
      value = _("Keywords");
      break;
    case FILE:
      value = _("File");
      break;
    case TSV:
      value = _("tsv");
      break;
    case NONE:
      value = _("None");
      break;
    default:
      assert(0);
  }

  return value;
}
