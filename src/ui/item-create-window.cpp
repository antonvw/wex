////////////////////////////////////////////////////////////////////////////////
// Name:      item.cpp
// Purpose:   Implementation of wex::item class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "item.h"
#include "ui.h"

namespace wex
{
template <typename S, typename T>
T* create_spinctrl(wxWindow* parent, const data::item& data, bool readonly)
{
  auto* window = new T(
    parent,
    data.window().id(),
    wxEmptyString,
    data.window().pos(),
    data.window().size(),
    data.window().style() | (readonly ? wxTE_READONLY : 0),
    std::any_cast<S>(data.min()),
    std::any_cast<S>(data.max()),
    !data.initial().has_value() ? std::any_cast<S>(data.min()) :
                                  std::any_cast<S>(data.initial()));

#ifndef __WXMSW__
  window->SetIncrement(std::any_cast<S>(data.inc()));
#endif

  return window;
}

wxArrayString
initial(const data::item& data, std::function<void(wxArrayString& as)> f)
{
  wxArrayString as;

  if (data.initial().has_value())
  {
    f(as);
  }

  return as;
}
} // namespace wex

bool wex::item::create_window(wxWindow* parent, bool readonly)
{
  if (m_type != USER && m_window != nullptr)
  {
    return false;
  }

  switch (m_type)
  {
    case EMPTY:
    case SPACER:
      break;

    case BUTTON:
      // Using a label is necessary for wxGTK.
      m_window = new wxButton(
        parent,
        m_data.window().id(),
        "default",
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style());
      (reinterpret_cast<wxButton*>(m_window))->SetLabelMarkup(m_label_window);
      break;

    case CHECKBOX:
      m_window = new wxCheckBox(
        parent,
        m_data.window().id(),
        m_label_window,
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style());

      if (m_data.initial().has_value())
      {
        (reinterpret_cast<wxCheckBox*>(m_window))
          ->SetValue(std::any_cast<bool>(m_data.initial()));
      }
      break;

    case CHECKLISTBOX_BIT:
    {
      auto* clb = new wxCheckListBox(
        parent,
        m_data.window().id(),
        m_data.window().pos(),
        m_data.window().size(),
        initial(
          m_data,
          [&](wxArrayString& as)
          {
            for (const auto& it : std::any_cast<choices_t>(m_data.initial()))
            {
              as.Add(after(before(it.second, ','), '.', false));
            }
          }),
        m_data.window().style());

      size_t item = 0;

      for (const auto& it : std::any_cast<choices_t>(m_data.initial()))
      {
        if (after(it.second, ',') == "1")
        {
          clb->Check(item);
        }

        item++;
      }
      m_window = clb;
    }
    break;

    case CHECKLISTBOX_BOOL:
    {
      auto* clb = new wxCheckListBox(
        parent,
        m_data.window().id(),
        m_data.window().pos(),
        m_data.window().size(),
        initial(
          m_data,
          [&](wxArrayString& as)
          {
            for (const auto& it :
                 std::any_cast<choices_bool_t>(m_data.initial()))
            {
              as.Add(after(before(it, ','), '.', false));
            }
          }),
        m_data.window().style());

      size_t item = 0;

      for (const auto& c : std::any_cast<choices_bool_t>(m_data.initial()))
      {
        if (after(c, ',') == "1")
        {
          clb->Check(item);
        }

        item++;
      }
      m_window = clb;
    }
    break;

    case COLOURPICKERWIDGET:
      m_window = new wxColourPickerWidget(
        parent,
        m_data.window().id(),
        m_data.initial().has_value() ?
          std::any_cast<wxColour>(m_data.initial()) :
          *wxBLACK,
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style() == data::NUMBER_NOT_SET ?
          wxCLRP_DEFAULT_STYLE :
          m_data.window().style());
      break;

    case COMBOBOX:
    case COMBOBOX_DIR:
    case COMBOBOX_FILE:
      m_window = new wxComboBox(
        parent,
        m_data.window().id(),
        wxEmptyString,
        m_data.window().pos(),
        m_data.window().size(),
        initial(
          m_data,
          [&](wxArrayString& as)
          {
            for (const auto& it :
                 std::any_cast<config::strings_t>(m_data.initial()))
            {
              as.Add(it);
            }
          }),
        m_data.window().style());
      break;

    case COMMANDLINKBUTTON:
      m_window = new wxCommandLinkButton(
        parent,
        m_data.window().id(),
        before(m_label_window, '\t'),
        after(m_label_window, '\t'),
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style());
      break;

    case DIRPICKERCTRL:
    {
      auto* pc = new wxDirPickerCtrl(
        parent,
        m_data.window().id(),
        !m_data.initial().has_value() ?
          std::string() :
          std::any_cast<std::string>(m_data.initial()),
        wxDirSelectorPromptStr,
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style() == data::NUMBER_NOT_SET ?
          wxDIRP_DEFAULT_STYLE :
          m_data.window().style());

      m_window = pc;

      if (pc->GetTextCtrl() != nullptr && readonly)
      {
        pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
      }
    }
    break;

    case FILEPICKERCTRL:
    {
#if defined(__WXMSW__)
      const std::string wc("*.exe");
#else // Unix/Mac
      const std::string wc(wxFileSelectorDefaultWildcardStr);
#endif

      auto* pc = new wxFilePickerCtrl(
        parent,
        m_data.window().id(),
        !m_data.initial().has_value() ?
          std::string() :
          std::any_cast<std::string>(m_data.initial()),
        wxFileSelectorPromptStr,
        wc,
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style() == data::NUMBER_NOT_SET ?
          wxFLP_DEFAULT_STYLE :
          m_data.window().style());

      m_window = pc;

      if (pc->GetTextCtrl() != nullptr && readonly)
      {
        pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
      }
    }
    break;

    case FONTPICKERCTRL:
    {
      auto* pc = new wxFontPickerCtrl(
        parent,
        m_data.window().id(),
        wxNullFont,
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style() == data::NUMBER_NOT_SET ?
          wxFNTP_DEFAULT_STYLE :
          m_data.window().style());

      m_window = pc;
      pc->SetPickerCtrlGrowable();

      if (pc->GetTextCtrl() != nullptr && readonly)
      {
        pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
      }
    }
    break;

    case GRID:
    {
      auto win(m_data.window());
      win.parent(parent);
      auto* gr = new grid(win);
      gr->CreateGrid(0, 0);
      gr->AppendCols(26);
      gr->AppendRows(100);
      m_window = gr;
    }
    break;

    case HYPERLINKCTRL:
      m_window = new wxHyperlinkCtrl(
        parent,
        m_data.window().id(),
        m_label_window,
        std::any_cast<std::string>(m_data.initial()),
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style() == data::NUMBER_NOT_SET ?
          wxHL_DEFAULT_STYLE :
          m_data.window().style());
      break;

    case LISTVIEW:
    {
      auto* lv = new listview(
        m_data_listview.window(data::window(m_data.window()).parent(parent)));
      lv->load(
        !m_data.initial().has_value() ?
          config::strings_t() :
          std::any_cast<config::strings_t>(m_data.initial()));
      m_window = lv;
    }
    break;

    case NOTEBOOK:
      create_book_control<wxNotebook>(parent, m_window, m_data);
      break;

    case NOTEBOOK_AUI:
      if (m_data.window().style() == data::NUMBER_NOT_SET)
      {
        data::window style(m_data.window());
        style.style(wxAUI_NB_DEFAULT_STYLE);
        m_data.window(style);
      }
      create_book_control<wxAuiNotebook>(parent, m_window, m_data);
      break;

    case NOTEBOOK_CHOICE:
      create_book_control<wxChoicebook>(parent, m_window, m_data);
      break;

    case NOTEBOOK_LIST:
      create_book_control<wxListbook>(parent, m_window, m_data);
      break;

    case NOTEBOOK_SIMPLE:
      create_book_control<wxSimplebook>(parent, m_window, m_data);
      break;

    case NOTEBOOK_TOOL:
      create_book_control<wxToolbook>(parent, m_window, m_data);
      break;

    case NOTEBOOK_TREE:
      create_book_control<wxTreebook>(parent, m_window, m_data);
      break;

    case NOTEBOOK_WEX:
      m_window =
        new notebook(m_data.window(data::window().parent(parent)).window());
      break;

    case RADIOBOX:
    {
      auto* rb = new wxRadioBox(
        parent,
        m_data.window().id(),
        m_label_window,
        m_data.window().pos(),
        m_data.window().size(),
        initial(
          m_data,
          [&](wxArrayString& as)
          {
            for (const auto& it : std::any_cast<choices_t>(m_data.initial()))
            {
              as.Add(before(it.second, ','));
            }
          }),
        m_data.columns(),
        m_data.window().style());

      size_t item = 0;

      for (const auto& c : std::any_cast<choices_t>(m_data.initial()))
      {
        if (after(c.second, ',') == "1")
        {
          rb->SetSelection(item);
        }

        item++;
      }
      m_window = rb;
    }
    break;

    case SLIDER:
      m_window = new wxSlider(
        parent,
        m_data.window().id(),
        std::any_cast<int>(m_data.initial()),
        std::any_cast<int>(m_data.min()),
        std::any_cast<int>(m_data.max()),
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style());
      break;

    case SPINCTRL:
      m_window = create_spinctrl<int, wxSpinCtrl>(parent, m_data, readonly);
      break;

    case SPINCTRLDOUBLE:
      m_window =
        create_spinctrl<double, wxSpinCtrlDouble>(parent, m_data, readonly);
      break;

    case STATICBOX:
      m_window = new wxStaticBox(parent, m_data.window().id(), m_label_window);
      break;

    case STATICLINE:
      m_window = new wxStaticLine(
        parent,
        m_data.window().id(),
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style());
      break;

    case STATICTEXT:
      m_window = new wxStaticText(
        parent,
        m_data.window().id(),
        wxEmptyString,
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style());
      (reinterpret_cast<wxStaticText*>(m_window))
        ->SetLabelMarkup(m_label_window);
      break;

    case TEXTCTRL:
    case TEXTCTRL_FLOAT:
    case TEXTCTRL_INT:
      m_window = new wxTextCtrl(
        parent,
        m_data.window().id(),
        !m_data.initial().has_value() ?
          std::string() :
          std::any_cast<std::string>(m_data.initial()),
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style() | (readonly ? wxTE_READONLY : 0) |
          (m_type == TEXTCTRL_FLOAT || m_type == TEXTCTRL_INT ? wxTE_RIGHT :
                                                                0));
      break;

    case TOGGLEBUTTON:
      m_window = new wxToggleButton(
        parent,
        m_data.window().id(),
        m_label_window,
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style());
      break;

    case USER:
      if (m_data.user_window_create() != nullptr)
      {
        assert(m_window != nullptr);
        (m_data.user_window_create())(m_window, parent, readonly);
      }
      break;

    default:
      assert(0);
  }

  if (m_data.control().validator() != nullptr)
  {
    m_window->SetValidator(*m_data.control().validator());
  }
  else if (m_type == TEXTCTRL_FLOAT)
  {
    m_window->SetValidator(wxFloatingPointValidator<double>());
  }
  else if (m_type == TEXTCTRL_INT)
  {
    m_window->SetValidator(wxIntegerValidator<int>());
  }

  if (!empty() && m_type != SPACER)
  {
    assert(m_window != nullptr);
  }

  return true;
}
