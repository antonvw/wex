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

void create_button(wxWindow* parent, wxWindow*& window, const wex::item& item)
{
  // Using a label is necessary for wxGTK.
  window = new wxButton(
    parent,
    item.data().window().id(),
    "default",
    item.data().window().pos(),
    item.data().window().size(),
    item.data().window().style());

  (reinterpret_cast<wxButton*>(window))->SetLabelMarkup(item.label_window());
}

void create_checkbox(wxWindow* parent, wxWindow*& window, const wex::item& item)
{
  window = new wxCheckBox(
    parent,
    item.data().window().id(),
    item.label_window(),
    item.data().window().pos(),
    item.data().window().size(),
    item.data().window().style());

  if (item.data().initial().has_value())
  {
    (reinterpret_cast<wxCheckBox*>(window))
      ->SetValue(std::any_cast<bool>(item.data().initial()));
  }
}

void create_checklistbox_bit(
  wxWindow*         parent,
  wxWindow*&        window,
  const data::item& data)
{
  auto* clb = new wxCheckListBox(
    parent,
    data.window().id(),
    data.window().pos(),
    data.window().size(),
    initial(
      data,
      [&](wxArrayString& as)
      {
        for (const auto& it : std::any_cast<item::choices_t>(data.initial()))
        {
          as.Add(after(before(it.second, ','), '.', false));
        }
      }),
    data.window().style());

  size_t item = 0;

  for (const auto& it : std::any_cast<item::choices_t>(data.initial()))
  {
    if (after(it.second, ',') == "1")
    {
      clb->Check(item);
    }

    item++;
  }

  window = clb;
}

void create_checklistbox_bool(
  wxWindow*         parent,
  wxWindow*&        window,
  const data::item& data)
{
  auto* clb = new wxCheckListBox(
    parent,
    data.window().id(),
    data.window().pos(),
    data.window().size(),
    initial(
      data,
      [&](wxArrayString& as)
      {
        for (const auto& it :
             std::any_cast<item::choices_bool_t>(data.initial()))
        {
          as.Add(after(before(it, ','), '.', false));
        }
      }),
    data.window().style());

  size_t item = 0;

  for (const auto& c : std::any_cast<item::choices_bool_t>(data.initial()))
  {
    if (after(c, ',') == "1")
    {
      clb->Check(item);
    }

    item++;
  }

  window = clb;
}

void create_combobox(
  wxWindow*         parent,
  wxWindow*&        window,
  const data::item& data)
{
  window = new wxComboBox(
    parent,
    data.window().id(),
    wxEmptyString,
    data.window().pos(),
    data.window().size(),
    initial(
      data,
      [&](wxArrayString& as)
      {
        for (const auto& it : std::any_cast<config::strings_t>(data.initial()))
        {
          as.Add(it);
        }
      }),
    data.window().style());
}

void create_colour_picket_widget(
  wxWindow*         parent,
  wxWindow*&        window,
  const data::item& data)
{
  window = new wxColourPickerWidget(
    parent,
    data.window().id(),
    data.initial().has_value() ? std::any_cast<wxColour>(data.initial()) :
                                 *wxBLACK,
    data.window().pos(),
    data.window().size(),
    data.window().style() == data::NUMBER_NOT_SET ? wxCLRP_DEFAULT_STYLE :
                                                    data.window().style());
}

void create_commandlink_button(
  wxWindow*        parent,
  wxWindow*&       window,
  const wex::item& item)
{
  window = new wxCommandLinkButton(
    parent,
    item.data().window().id(),
    before(item.label_window(), '\t'),
    after(item.label_window(), '\t'),
    item.data().window().pos(),
    item.data().window().size(),
    item.data().window().style());
}

void create_dir_picker_control(
  wxWindow*         parent,
  wxWindow*&        window,
  const data::item& data)
{
  auto* pc = new wxDirPickerCtrl(
    parent,
    data.window().id(),
    !data.initial().has_value() ? std::string() :
                                  std::any_cast<std::string>(data.initial()),
    wxDirSelectorPromptStr,
    data.window().pos(),
    data.window().size(),
    data.window().style() == data::NUMBER_NOT_SET ? wxDIRP_DEFAULT_STYLE :
                                                    data.window().style());

  window = pc;

  if (pc->GetTextCtrl() != nullptr && data.is_readonly())
  {
    pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
  }
}

void create_file_picker_control(
  wxWindow*         parent,
  wxWindow*&        window,
  const data::item& data)
{
#if defined(__WXMSW__)
  const std::string wc("*.exe");
#else // Unix/Mac
  const std::string wc(wxFileSelectorDefaultWildcardStr);
#endif

  auto* pc = new wxFilePickerCtrl(
    parent,
    data.window().id(),
    !data.initial().has_value() ? std::string() :
                                  std::any_cast<std::string>(data.initial()),
    wxFileSelectorPromptStr,
    wc,
    data.window().pos(),
    data.window().size(),
    data.window().style() == data::NUMBER_NOT_SET ? wxFLP_DEFAULT_STYLE :
                                                    data.window().style());

  window = pc;

  if (pc->GetTextCtrl() != nullptr && data.is_readonly())
  {
    pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
  }
}

void create_font_picker_control(
  wxWindow*         parent,
  wxWindow*&        window,
  const data::item& data)
{
  auto* pc = new wxFontPickerCtrl(
    parent,
    data.window().id(),
    wxNullFont,
    data.window().pos(),
    data.window().size(),
    data.window().style() == data::NUMBER_NOT_SET ? wxFNTP_DEFAULT_STYLE :
                                                    data.window().style());

  window = pc;
  pc->SetPickerCtrlGrowable();

  if (pc->GetTextCtrl() != nullptr && data.is_readonly())
  {
    pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
  }
}

void create_grid_control(
  wxWindow*         parent,
  wxWindow*&        window,
  const data::item& data)
{
  auto win(data.window());
  win.parent(parent);
  auto* gr = new grid(win);
  
  gr->CreateGrid(0, 0);
  gr->AppendCols(26);
  gr->AppendRows(100);
  
  window = gr;
}

void create_hyperlink_control(
  wxWindow*        parent,
  wxWindow*&       window,
  const wex::item& item)
{
  window = new wxHyperlinkCtrl(
    parent,
    item.data().window().id(),
    item.label_window(),
    std::any_cast<std::string>(item.data().initial()),
    item.data().window().pos(),
    item.data().window().size(),
    item.data().window().style() == data::NUMBER_NOT_SET ?
      wxHL_DEFAULT_STYLE :
      item.data().window().style());
}

void create_listview(wxWindow* parent, wxWindow*& window, const wex::item& item)
{
  auto* lv = new listview(item.data_listview());

  lv->load(
    !item.data().initial().has_value() ?
      config::strings_t() :
      std::any_cast<config::strings_t>(item.data().initial()));

  window = lv;
}

void create_radiobox(wxWindow* parent, wxWindow*& window, const wex::item& item)
{
  auto* rb = new wxRadioBox(
    parent,
    item.data().window().id(),
    item.label_window(),
    item.data().window().pos(),
    item.data().window().size(),
    initial(
      item.data(),
      [&](wxArrayString& as)
      {
        for (const auto& it :
             std::any_cast<item::choices_t>(item.data().initial()))
        {
          as.Add(before(it.second, ','));
        }
      }),
    item.data().columns(),
    item.data().window().style());

  size_t item_no = 0;

  for (const auto& c : std::any_cast<item::choices_t>(item.data().initial()))
  {
    if (after(c.second, ',') == "1")
    {
      rb->SetSelection(item_no);
    }

    item_no++;
  }

  window = rb;
}

void create_slider(wxWindow* parent, wxWindow*& window, const data::item& data)
{
  window = new wxSlider(
    parent,
    data.window().id(),
    std::any_cast<int>(data.initial()),
    std::any_cast<int>(data.min()),
    std::any_cast<int>(data.max()),
    data.window().pos(),
    data.window().size(),
    data.window().style());
}

template <typename S, typename T>
void create_spinctrl(
  wxWindow*         parent,
  wxWindow*&        window,
  const data::item& data)
{
  auto* ctrl = new T(
    parent,
    data.window().id(),
    wxEmptyString,
    data.window().pos(),
    data.window().size(),
    data.window().style() | (data.is_readonly() ? wxTE_READONLY : 0),
    std::any_cast<S>(data.min()),
    std::any_cast<S>(data.max()),
    !data.initial().has_value() ? std::any_cast<S>(data.min()) :
                                  std::any_cast<S>(data.initial()));

#ifndef __WXMSW__
  ctrl->SetIncrement(std::any_cast<S>(data.inc()));
#endif

  window = ctrl;
}

void create_staticline(
  wxWindow*         parent,
  wxWindow*&        window,
  const data::item& data)
{
  window = new wxStaticLine(
    parent,
    data.window().id(),
    data.window().pos(),
    data.window().size(),
    data.window().style());
}

void create_statictext(
  wxWindow*        parent,
  wxWindow*&       window,
  const wex::item& item)
{
  window = new wxStaticText(
    parent,
    item.data().window().id(),
    wxEmptyString,
    item.data().window().pos(),
    item.data().window().size(),
    item.data().window().style());

  (reinterpret_cast<wxStaticText*>(window))
    ->SetLabelMarkup(item.label_window());
}

void create_textctrl(wxWindow* parent, wxWindow*& window, const wex::item& item)
{
  window = new wxTextCtrl(
    parent,
    item.data().window().id(),
    !item.data().initial().has_value() ?
      std::string() :
      std::any_cast<std::string>(item.data().initial()),
    item.data().window().pos(),
    item.data().window().size(),
    item.data().window().style() |
      (item.data().is_readonly() ? wxTE_READONLY : 0) |
      (item.type() == item::TEXTCTRL_FLOAT ||
           item.type() == item::TEXTCTRL_INT ?
         wxTE_RIGHT :
         0));
}

void create_togglebutton(
  wxWindow*        parent,
  wxWindow*&       window,
  const wex::item& item)
{
  window = new wxToggleButton(
    parent,
    item.data().window().id(),
    item.label_window(),
    item.data().window().pos(),
    item.data().window().size(),
    item.data().window().style());
}

void set_validator(wex::item* item)
{
  if (item->data().control().validator() != nullptr)
  {
    item->window()->SetValidator(*item->data().control().validator());
  }
  else if (item->type() == item::TEXTCTRL_FLOAT)
  {
    item->window()->SetValidator(wxFloatingPointValidator<double>());
  }
  else if (item->type() == item::TEXTCTRL_INT)
  {
    item->window()->SetValidator(wxIntegerValidator<int>());
  }

  if (!item->empty() && item->type() != item::SPACER)
  {
    assert(item->window() != nullptr);
  }
}
} // namespace wex

bool wex::item::create_window(wxWindow* parent, bool readonly)
{
  if (m_type != USER && m_window != nullptr)
  {
    return false;
  }

  m_data.is_readonly(readonly);

  switch (m_type)
  {
    case EMPTY:
    case SPACER:
      break;

    case BUTTON:
      create_button(parent, m_window, *this);
      break;

    case CHECKBOX:
      create_checkbox(parent, m_window, *this);
      break;

    case CHECKLISTBOX_BIT:
      create_checklistbox_bit(parent, m_window, m_data);
      break;

    case CHECKLISTBOX_BOOL:
      create_checklistbox_bool(parent, m_window, m_data);
      break;

    case COLOURPICKERWIDGET:
      create_colour_picket_widget(parent, m_window, m_data);
      break;

    case COMBOBOX:
    case COMBOBOX_DIR:
    case COMBOBOX_FILE:
      create_combobox(parent, m_window, m_data);
      break;

    case COMMANDLINKBUTTON:
      create_commandlink_button(parent, m_window, *this);
      break;

    case DIRPICKERCTRL:
      create_dir_picker_control(parent, m_window, m_data);
      break;

    case FILEPICKERCTRL:
      create_file_picker_control(parent, m_window, m_data);
      break;

    case FONTPICKERCTRL:
      create_font_picker_control(parent, m_window, m_data);
      break;

    case GRID:
      create_grid_control(parent, m_window, m_data);
      break;

    case HYPERLINKCTRL:
      create_hyperlink_control(parent, m_window, *this);
      break;

    case LISTVIEW:
      m_data_listview.window(data::window(m_data.window()).parent(parent));
      create_listview(parent, m_window, *this);
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
      create_radiobox(parent, m_window, *this);
      break;

    case SLIDER:
      create_slider(parent, m_window, m_data);
      break;

    case SPINCTRL:
      create_spinctrl<int, wxSpinCtrl>(parent, m_window, m_data);
      break;

    case SPINCTRLDOUBLE:
      create_spinctrl<double, wxSpinCtrlDouble>(parent, m_window, m_data);
      break;

    case STATICBOX:
      m_window = new wxStaticBox(parent, m_data.window().id(), m_label_window);
      break;

    case STATICLINE:
      create_staticline(parent, m_window, m_data);
      break;

    case STATICTEXT:
      create_statictext(parent, m_window, *this);
      break;

    case TEXTCTRL:
    case TEXTCTRL_FLOAT:
    case TEXTCTRL_INT:
      create_textctrl(parent, m_window, *this);
      break;

    case TOGGLEBUTTON:
      create_togglebutton(parent, m_window, *this);
      break;

    case USER:
      if (m_data.user_window_create() != nullptr)
      {
        assert(m_window != nullptr);
        (m_data.user_window_create())(m_window, parent);
      }
      break;

    default:
      assert(0);
  }

  set_validator(this);

  return true;
}
