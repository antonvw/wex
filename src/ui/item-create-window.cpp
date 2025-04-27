////////////////////////////////////////////////////////////////////////////////
// Name:      item-create-window.cpp
// Purpose:   Implementation of wex::item::create_window and wex::item::creators
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <utility>

#include "item.h"
#include "ui.h"

#define CREATE_CTRL(CONTROL)                                                   \
  {[&](wxWindow* parent, wxWindow*& window, const wex::item& item)             \
   {                                                                           \
     CONTROL(parent, window, item);                                            \
   }},

#define DET_TEXT()                                                             \
  std::string text;                                                            \
                                                                               \
  if constexpr (std::is_same_v<T, wex::item::choices_t>)                       \
  {                                                                            \
    text = it.second;                                                          \
  }                                                                            \
  else                                                                         \
  {                                                                            \
    text = it;                                                                 \
  }

#define IPS                                                                    \
  item.data().window().id(), item.data().window().pos(),                       \
    item.data().window().size()

#define PII                                                                    \
  parent, item.data().window().id(),                                           \
    !item.data().initial().has_value() ?                                       \
      std::string() :                                                          \
      std::any_cast<std::string>(item.data().initial())

#define PIL parent, item.data().window().id(), item.label_window()

#define PS item.data().window().pos(), item.data().window().size()

#define PSS PS, item.data().window().style()

#define CHOICE                                                                 \
  initial(                                                                     \
    item.data(),                                                               \
    [&](wxArrayString& as)                                                     \
    {                                                                          \
      for (const auto& it :                                                    \
           std::any_cast<config::strings_t>(item.data().initial()))            \
      {                                                                        \
        as.Add(it);                                                            \
      }                                                                        \
    }),                                                                        \
    item.data().window().style()

namespace wex
{
void finish_picker(wxPickerBase* pc, const wex::item& item, wxWindow*& window)
{
  window = pc;

  pc->SetPickerCtrlGrowable();

  if (pc->GetTextCtrl() != nullptr && item.data().is_readonly())
  {
    pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
  }
}

void handle(const std::string& s, wxCheckListBox* clb, size_t& item_no)
{
  if (find_after(s, ",") == "1")
  {
    clb->Check(item_no);
  }

  item_no++;
}

auto initial(
  const data::item&                             data,
  const std::function<void(wxArrayString& as)>& f)
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
  window = new wxButton(parent, item.data().window().id(), "default", PSS);

  (reinterpret_cast<wxButton*>(window))->SetLabelMarkup(item.label_window());
}

void create_checkbox(wxWindow* parent, wxWindow*& window, const wex::item& item)
{
  window = new wxCheckBox(PIL, PSS);

  if (item.data().initial().has_value())
  {
    (reinterpret_cast<wxCheckBox*>(window))
      ->SetValue(std::any_cast<bool>(item.data().initial()));
  }
}

auto* create_checklistbox(
  wxWindow*                                     parent,
  const wex::item&                              item,
  const std::function<void(wxArrayString& as)>& f)
{
  return new wxCheckListBox(
    parent,
    IPS,
    initial(item.data(), std::move(f)),
    item.data().window().style());
}

template <typename T>
void create_checklistbox_as(
  wxWindow*        parent,
  wxWindow*&       window,
  const wex::item& item)
{
  auto* clb = create_checklistbox(
    parent,
    item,
    [&](wxArrayString& as)
    {
      for (const auto& it : std::any_cast<T>(item.data().initial()))
      {
        DET_TEXT()
        as.Add(rfind_after(find_before(text, ","), "."));
      }
    });

  for (size_t      item_no = 0;
       const auto& it : std::any_cast<T>(item.data().initial()))
  {
    DET_TEXT()
    handle(text, clb, item_no);
  }

  window = clb;
}

void create_checklistbox_bit(
  wxWindow*        parent,
  wxWindow*&       window,
  const wex::item& item)
{
  create_checklistbox_as<wex::item::choices_t>(parent, window, item);
}

void create_checklistbox_bool(
  wxWindow*        parent,
  wxWindow*&       window,
  const wex::item& item)
{
  create_checklistbox_as<wex::item::choices_bool_t>(parent, window, item);
}

void create_combobox(wxWindow* parent, wxWindow*& window, const wex::item& item)
{
  window = new wxComboBox(PIL, PS, CHOICE);
}

void create_colour_picket_widget(
  wxWindow*        parent,
  wxWindow*&       window,
  const wex::item& item)
{
  window = new wxColourPickerWidget(
    parent,
    item.data().window().id(),
    item.data().initial().has_value() ?
      std::any_cast<wxColour>(item.data().initial()) :
      *wxBLACK,
    PSS == data::NUMBER_NOT_SET ? wxCLRP_DEFAULT_STYLE :
                                  item.data().window().style());
}

void create_commandlink_button(
  wxWindow*        parent,
  wxWindow*&       window,
  const wex::item& item)
{
  window = new wxCommandLinkButton(
    parent,
    item.data().window().id(),
    find_before(item.label_window(), "\t"),
    find_after(item.label_window(), "\t"),
    PSS);
}

void create_listbox(wxWindow* parent, wxWindow*& window, const wex::item& item)
{
  window = new wxListBox(parent, IPS, CHOICE);
}

void create_dir_picker_control(
  wxWindow*        parent,
  wxWindow*&       window,
  const wex::item& item)
{
  auto* pc = new wxDirPickerCtrl(
    PII,
    wxDirSelectorPromptStr,
    PSS == data::NUMBER_NOT_SET ? wxDIRP_DEFAULT_STYLE :
                                  item.data().window().style());

  finish_picker(pc, item, window);
}

void create_empty_or_spacer_control(
  wxWindow*        parent,
  wxWindow*&       window,
  const wex::item& item)
{
  window = nullptr;
}

void create_file_picker_control(
  wxWindow*        parent,
  wxWindow*&       window,
  const wex::item& item)
{
#if defined(__WXMSW__)
  const std::string wc("*.exe");
#else // Unix/Mac
  const std::string wc(wxFileSelectorDefaultWildcardStr);
#endif

  auto* pc = new wxFilePickerCtrl(
    PII,
    wxFileSelectorPromptStr,
    wc,
    PSS == data::NUMBER_NOT_SET ? wxFLP_DEFAULT_STYLE | wxFLP_SMALL :
                                  item.data().window().style());

  finish_picker(pc, item, window);
}

void create_font_picker_control(
  wxWindow*        parent,
  wxWindow*&       window,
  const wex::item& item)
{
  auto* pc = new wxFontPickerCtrl(
    parent,
    item.data().window().id(),
    std::any_cast<wxFont>(item.data().initial()),
    PSS == data::NUMBER_NOT_SET ? wxFNTP_DEFAULT_STYLE | wxPB_SMALL :
                                  item.data().window().style());

  finish_picker(pc, item, window);
}

void create_grid_control(
  wxWindow*        parent,
  wxWindow*&       window,
  const wex::item& item)
{
  auto win(item.data().window());
  win.parent(parent);
  auto* gr = new grid(win);

  gr->CreateGrid(10, item.data().columns());
  gr->ShowScrollbars(wxSHOW_SB_ALWAYS, wxSHOW_SB_ALWAYS);

  window = gr;
}

void create_hyperlink_control(
  wxWindow*        parent,
  wxWindow*&       window,
  const wex::item& item)
{
  window = new wxHyperlinkCtrl(
    PIL,
    std::any_cast<std::string>(item.data().initial()),
    PSS == data::NUMBER_NOT_SET ? wxHL_DEFAULT_STYLE :
                                  item.data().window().style());
}

void create_panel(wxWindow* parent, wxWindow*& window, const wex::item& item)
{
  window = new wxPanel(parent, item.data().window().id());
}

void create_radiobox(wxWindow* parent, wxWindow*& window, const wex::item& item)
{
  auto* rb = new wxRadioBox(
    PIL,
    PS,
    initial(
      item.data(),
      [&](wxArrayString& as)
      {
        for (const auto& it :
             std::any_cast<item::choices_t>(item.data().initial()))
        {
          as.Add(find_before(it.second, ","));
        }
      }),
    item.data().columns(),
    item.data().window().style());

  size_t item_no = 0;

  for (const auto& c : std::any_cast<item::choices_t>(item.data().initial()))
  {
    if (find_after(c.second, ",") == "1")
    {
      rb->SetSelection(item_no);
    }

    item_no++;
  }

  window = rb;
}

void create_slider(wxWindow* parent, wxWindow*& window, const wex::item& item)
{
  window = new wxSlider(
    parent,
    item.data().window().id(),
    std::any_cast<int>(item.data().initial()),
    std::any_cast<int>(item.data().min()),
    std::any_cast<int>(item.data().max()),
    PSS);
}

template <typename S, typename T>
void create_spinctrl(wxWindow* parent, wxWindow*& window, const wex::item& item)
{
  auto* ctrl = new T(
    parent,
    item.data().window().id(),
    wxEmptyString,
    PSS | (item.data().is_readonly() ? wxTE_READONLY : 0),
    std::any_cast<S>(item.data().min()),
    std::any_cast<S>(item.data().max()),
    !item.data().initial().has_value() ?
      std::any_cast<S>(item.data().min()) :
      std::any_cast<S>(item.data().initial()));

#ifndef __WXMSW__
  ctrl->SetIncrement(std::any_cast<S>(item.data().inc()));
#endif

  window = ctrl;
}

void create_staticbox(
  wxWindow*        parent,
  wxWindow*&       window,
  const wex::item& item)
{
  window = new wxStaticBox(PIL);
}

void create_staticline(
  wxWindow*        parent,
  wxWindow*&       window,
  const wex::item& item)
{
  window = new wxStaticLine(parent, item.data().window().id(), PSS);
}

void create_statictext(
  wxWindow*        parent,
  wxWindow*&       window,
  const wex::item& item)
{
  window =
    new wxStaticText(parent, item.data().window().id(), wxEmptyString, PSS);

  (reinterpret_cast<wxStaticText*>(window))
    ->SetLabelMarkup(item.label_window());
}

void create_textctrl(wxWindow* parent, wxWindow*& window, const wex::item& item)
{
  window = new wxTextCtrl(
    PII,
    PSS | (item.data().is_readonly() ? wxTE_READONLY : 0) |
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
  window = new wxToggleButton(PIL, PSS);
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
}
} // namespace wex

bool wex::item::create_window(wxWindow* parent, bool readonly)
{
  if (m_type != USER && m_window != nullptr)
  {
    return false;
  }

  if (m_creators.empty())
  {
    m_creators = creators();
  }

  assert(m_type < m_creators.size());

  m_data.is_readonly(readonly);

  m_creators[m_type](parent, m_window, *this);

  assert(
    m_type == EMPTY || m_type == SPACER ? m_window == nullptr :
                                          m_window != nullptr);

  set_validator(this);

  return true;
}

wex::item::create_t wex::item::creators()
{
  // these creators should exactly follow the item_t enum
  // clang-format off
  return {
    CREATE_CTRL(create_button) 
    CREATE_CTRL(create_checkbox) 
    CREATE_CTRL(create_checklistbox_bit) 
    CREATE_CTRL(create_checklistbox_bool) 
    CREATE_CTRL(create_colour_picket_widget)
    CREATE_CTRL(create_combobox) 
    CREATE_CTRL(create_combobox) 
    CREATE_CTRL(create_combobox) 
    CREATE_CTRL(create_commandlink_button)
    CREATE_CTRL(create_dir_picker_control)
    CREATE_CTRL(create_empty_or_spacer_control) 
    CREATE_CTRL(create_file_picker_control) 
    CREATE_CTRL(create_font_picker_control)
    CREATE_CTRL(create_grid_control)
    CREATE_CTRL(create_panel) 
    CREATE_CTRL(create_hyperlink_control)
    CREATE_CTRL(create_listbox)
    {[&](wxWindow* parent, wxWindow*& window, const wex::item& item)
      {
        auto* lv = new listview(m_data_listview.window(
          data::window(m_data.window()).parent(parent)));
        lv->load(
          !item.data().initial().has_value() ?
            config::strings_t() :
            std::any_cast<config::strings_t>(item.data().initial()));
        window = lv;
      }
    },
    CREATE_CTRL(create_book_control<wxNotebook>)
    {[&](wxWindow* parent, wxWindow*& window, const wex::item& item)
      {
        if (m_data.window().style() == data::NUMBER_NOT_SET)
        {
          data::window style(m_data.window());
          style.style(notebook::default_style_t);
          m_data.window(style);
        }
      create_book_control<wxAuiNotebook>(parent, window, item);
      }
    },
    CREATE_CTRL(create_book_control<wxChoicebook>)
    CREATE_CTRL(create_book_control<wxListbook>)
    CREATE_CTRL(create_book_control<wxSimplebook>)
    CREATE_CTRL(create_book_control<wxToolbook>)
    CREATE_CTRL(create_book_control<wxTreebook>)
    {
      [&](wxWindow* parent, wxWindow*& window, const wex::item& item)
      {
        window = new notebook(
          m_data.window(data::window().parent(parent)).window());
      }
    },
    CREATE_CTRL(create_radiobox)
    CREATE_CTRL(create_slider)
    CREATE_CTRL(create_empty_or_spacer_control)
    {[&](wxWindow* parent, wxWindow*& window, const wex::item& item)
      {
        create_spinctrl<int, wxSpinCtrl>(parent, window, item);
      }},
    {[&](wxWindow* parent, wxWindow*& window, const wex::item& item)
     {
       create_spinctrl<double, wxSpinCtrlDouble>(parent, window, item);
     }},
    CREATE_CTRL(create_staticbox)
    CREATE_CTRL(create_staticline)
    CREATE_CTRL(create_statictext) 
    CREATE_CTRL(create_textctrl)
    CREATE_CTRL(create_textctrl) 
    CREATE_CTRL(create_textctrl)
    CREATE_CTRL(create_togglebutton)
    {[&](wxWindow* parent, wxWindow*& window, const wex::item& item)
      {
        if (m_data.user_window_create() != nullptr)
        {
          (m_data.user_window_create())(m_window, parent);
        }
      }}};
  // clang-format on
}
