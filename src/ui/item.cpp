////////////////////////////////////////////////////////////////////////////////
// Name:      item.cpp
// Purpose:   Implementation of wex::item class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/aui/auibook.h>
#include <wx/bookctrl.h>
#include <wx/button.h>
#include <wx/checklst.h>
#include <wx/choicebk.h>
#include <wx/clrpicker.h> // for wxColourPickerWidget
#include <wx/commandlinkbutton.h>
#include <wx/filepicker.h>
#include <wx/fontpicker.h>
#include <wx/hyperlink.h>
#include <wx/imaglist.h>
#include <wx/listbook.h>
#include <wx/notebook.h>
#include <wx/persist/treebook.h>
#include <wx/simplebook.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>
#include <wx/toolbook.h>
#include <wx/valnum.h>
#include <wx/valtext.h>
#include <wx/window.h>

#include <wex/common/tocontainer.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/common/util.h>
#include <wex/ui/grid.h>
#include <wex/ui/item-template-dialog.h>
#include <wex/ui/item.h>
#include <wex/ui/listview.h>
#include <wex/ui/notebook.h>

#include <sstream>

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

const item::type_t use_type(const std::string& label, item::type_t t)
{
  return label.find(':') != std::string::npos ? item::STATICTEXT : t;
}

const std::string str(const std::string& name, const std::any& any)
{
  std::stringstream s;

  s << name << "{internal type: " << any.type().name() << ", value: ";

  if (any.has_value())
  {
    try
    {
      if (any.type() == typeid(int))
      {
        s << std::any_cast<int>(any);
      }
      else if (any.type() == typeid(long))
      {
        s << std::any_cast<long>(any);
      }
      else if (any.type() == typeid(double))
      {
        s << std::any_cast<double>(any);
      }
      else if (any.type() == typeid(std::string))
      {
        s << std::any_cast<std::string>(any);
      }
      else
      {
        s << "<no cast available>";
      }
    }
    catch (std::bad_cast& e)
    {
      s << "<log bad cast: " << e.what() << ">";
    }
  }
  else
  {
    s << "<no value>";
  }

  s << "} ";

  return s.str();
}
} // namespace wex

wex::item::item(
  type_t             type,
  const std::string& label,
  const std::any&    value,
  const data::item&  data)
  : m_type(type)
  , m_data(data, value)
  , m_label(label)
  , m_label_window(after(label, '.', false))
  , m_sizer_flags(wxSizerFlags().Border().Left())
{
  switch (m_type)
  {
    case CHECKLISTBOX_BIT:
    case CHECKLISTBOX_BOOL:
    case GRID:
    case LISTVIEW:
    case NOTEBOOK:
    case NOTEBOOK_AUI:
    case NOTEBOOK_LIST:
    case NOTEBOOK_SIMPLE:
    case NOTEBOOK_TOOL:
    case NOTEBOOK_TREE:
    case NOTEBOOK_WEX:
    case RADIOBOX:
    case STATICBOX:
      m_is_row_growable = true;
      m_sizer_flags.Expand();
      break;

    case STATICTEXT:
    case TEXTCTRL:
    case TEXTCTRL_FLOAT:
    case TEXTCTRL_INT:
      m_is_row_growable = (m_data.window().style() & wxTE_MULTILINE) > 0;
      m_sizer_flags.Expand();
      break;

    case CHECKBOX:
    case COMBOBOX:
    case COMBOBOX_DIR:
    case COMBOBOX_FILE:
    case DIRPICKERCTRL:
    case FILEPICKERCTRL:
    case SPACER:
    case STATICLINE:
    case USER:
      m_sizer_flags.Expand();
      break;

    default:; // prevent warning
  }
}

wex::item::item(const std::string& label, type_t type, const data::item& data)
  : item(label, type, data.initial(), data)
{
}

wex::item::item(
  const std::string& label,
  const std::string& value,
  type_t             type,
  const data::item&  data)
  : item(
      use_type(label, type),
      label,
      value,
      data::item(data).label_type(
        (use_type(label, type) != STATICTEXT && type != HYPERLINKCTRL ?
           data.label_type() :
           data::item::LABEL_NONE)))
{
}

wxFlexGridSizer* wex::item::add(wxSizer* sizer, wxFlexGridSizer* current) const
{
  assert(m_window != nullptr);

  if (current == nullptr)
  {
    current = new wxFlexGridSizer(
      m_data.label_type() == data::item::LABEL_LEFT ? 2 : 1);
    current->AddGrowableCol(current->GetCols() - 1); // make last col growable
    sizer->Add(current, wxSizerFlags().Expand());
  }

  if (m_data.label_type() != data::item::LABEL_NONE && !m_label.empty())
  {
    add_static_text(current);
  }

  current->Add(m_window, m_sizer_flags);

  if (m_is_row_growable && current->GetEffectiveRowsCount() >= 1)
  {
    current->AddGrowableRow(current->GetEffectiveRowsCount() - 1);
  }

  return current;
}

wxFlexGridSizer* wex::item::add_browse_button(wxSizer* sizer) const
{
  assert(m_window != nullptr);

  auto* fgz = new wxFlexGridSizer(3);
  fgz->AddGrowableCol(1);

  add_static_text(fgz);

  fgz->Add(m_window, m_sizer_flags);

  // Tried to use a wxDirPickerCtrl here, is nice,
  // but does not use a combobox for keeping old values, so this is better.
  // And the text box that is used is not resizable as well.
  // In item_template_dialog the button click is bound to browse dialog.
  auto* browse = new wxButton(
    m_window->GetParent(),
    m_window->GetId(),
    _(wxDirPickerWidgetLabel));

  fgz->Add(browse, wxSizerFlags().Center().Border());

  sizer->Add(fgz, wxSizerFlags().Left().Expand()); // no border

  return fgz;
}

void wex::item::add_items(group_t& page, bool readonly)
{
  int use_cols = 1;
  if (m_data.columns() != -1)
    use_cols = m_data.columns();

  if (auto* bookctrl = dynamic_cast<wxBookCtrlBase*>(m_window);
      bookctrl != nullptr)
  {
    m_page      = page.first;
    int imageId = wxWithImages::NO_IMAGE;

    if (const auto col = m_page.find(":"); col != std::string::npos)
    {
      use_cols = std::stoi(m_page.substr(col + 1));
      m_page   = m_page.substr(0, col);
    }

    auto* fgz = new wxFlexGridSizer(use_cols);

    if (m_data.image_list() != nullptr)
    {
      if (
        static_cast<int>(bookctrl->GetPageCount()) <
        m_data.image_list()->GetImageCount())
      {
        imageId = bookctrl->GetPageCount();
      }
      else
      {
        log() << "more pages than images";
      }
    }

    bookctrl->AddPage(
      new wxWindow(bookctrl, wxID_ANY),
      m_page,
      true, // select
      imageId);

    bookctrl->GetCurrentPage()->SetSizer(fgz);

    for (int i = 0; i < use_cols; i++)
    {
      fgz->AddGrowableCol(i);
    }

    add_items(bookctrl->GetCurrentPage(), fgz, page.second, readonly);
  }
  else if (auto* sb = dynamic_cast<wxStaticBox*>(m_window); sb != nullptr)
  {
    auto* fgz = new wxFlexGridSizer(use_cols);
    add_items(sb, fgz, page.second, readonly);
    sb->SetSizerAndFit(fgz);
  }
  else
  {
    assert(0);
  }
}

void wex::item::add_items(
  wxWindow*          parent,
  wxFlexGridSizer*   fgz,
  std::vector<item>& v,
  bool               readonly)
{
  wxFlexGridSizer* previous_item_sizer = nullptr;
  int              previous_type       = -1;

  for (auto& item : v)
  {
    // If this item has same type as previous type use previous sizer,
    // otherwise use no sizer (layout will create a new one).
    wxFlexGridSizer* current_item_sizer =
      (item.type() == previous_type ? previous_item_sizer : nullptr);

    previous_item_sizer =
      item.layout(parent, fgz, readonly, current_item_sizer);

    previous_type = item.type();

    if (m_dialog != nullptr)
    {
      item.set_dialog(m_dialog);
      m_dialog->add(item);
      m_dialog->bind_button(item);
    }

    if (
      fgz->GetEffectiveRowsCount() >= 1 &&
      !fgz->IsRowGrowable(fgz->GetEffectiveRowsCount() - 1) &&
      item.is_row_growable())
    {
      fgz->AddGrowableRow(fgz->GetEffectiveRowsCount() - 1);
    }
  }
}

wxFlexGridSizer* wex::item::add_static_text(wxSizer* sizer) const
{
  assert(!m_label_window.empty());
  assert(m_window != nullptr);

  sizer->Add(
    new wxStaticText(m_window->GetParent(), wxID_ANY, m_label_window + ":"),
    wxSizerFlags().Right().Border().Align(wxALIGN_LEFT));

  return nullptr;
}

bool wex::item::apply(bool save) const
{
  if (m_data.apply() != nullptr)
  {
    (m_data.apply())(m_window, get_value(), save);
    return true;
  }

  return false;
}

bool wex::item::create_window(wxWindow* parent, bool readonly)
{
  if (m_type != USER && m_window != nullptr)
  {
    return false;
  }

  wxBookCtrlBase* bookctrl = nullptr;

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
      bookctrl = new wxNotebook(
        parent,
        m_data.window().id(),
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style());
      break;

    case NOTEBOOK_AUI:
      bookctrl = new wxAuiNotebook(
        parent,
        m_data.window().id(),
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style() == data::NUMBER_NOT_SET ?
          wxAUI_NB_DEFAULT_STYLE :
          m_data.window().style());
      break;

    case NOTEBOOK_CHOICE:
      bookctrl = new wxChoicebook(
        parent,
        m_data.window().id(),
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style());
      break;

    case NOTEBOOK_LIST:
      bookctrl = new wxListbook(
        parent,
        m_data.window().id(),
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style());
      break;

    case NOTEBOOK_SIMPLE:
      bookctrl = new wxSimplebook(
        parent,
        m_data.window().id(),
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style());
      break;

    case NOTEBOOK_TOOL:
      bookctrl = new wxToolbook(
        parent,
        m_data.window().id(),
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style());
      break;

    case NOTEBOOK_TREE:
      bookctrl = new wxTreebook(
        parent,
        m_data.window().id(),
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style());
      break;

    case NOTEBOOK_WEX:
      bookctrl =
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

  if (bookctrl != nullptr)
  {
    m_window = bookctrl;

    if (m_data.image_list() != nullptr)
    {
      bookctrl->SetImageList(m_data.image_list());
    }
    else if (m_type == NOTEBOOK_TOOL)
    {
      log() << "toolbook requires image list";
    }
  }

  if (!empty() && m_type != SPACER)
  {
    assert(m_window != nullptr);
  }

  return true;
}

const std::any wex::item::get_value() const
{
  if (m_window == nullptr)
  {
    switch (m_type)
    {
      case CHECKBOX:
        if (m_data.initial().has_value())
        {
          return std::any_cast<bool>(m_data.initial());
        }
        else
        {
          return false;
        }

      case CHECKLISTBOX_BIT:
      case RADIOBOX:
      {
        long value = 0;
        int  item  = 0;
        for (const auto& b : std::any_cast<choices_t>(m_data.initial()))
        {
          if (b.second.find(",") != std::string::npos)
          {
            value |= b.first;
          }
          item++;
        }
        return std::any(value);
      }

      default:
        return m_data.initial();
    }
  }

  std::any any;

  if (is_notebook())
  {
    return any;
  }

  try
  {
    switch (m_type)
    {
      case BUTTON:
        break;

      case CHECKBOX:
        any = (reinterpret_cast<wxCheckBox*>(m_window))->GetValue();
        break;

      case CHECKLISTBOX_BIT:
      {
        auto* clb   = reinterpret_cast<wxCheckListBox*>(m_window);
        long  value = 0;
        int   item  = 0;

        for (const auto& b : std::any_cast<choices_t>(m_data.initial()))
        {
          if (clb->IsChecked(item))
          {
            value |= b.first;
          }

          item++;
        }

        any = value;
      }
      break;

      case COLOURPICKERWIDGET:
        any = (reinterpret_cast<wxColourPickerWidget*>(m_window))->GetColour();
        break;

      case COMBOBOX:
      case COMBOBOX_DIR:
      case COMBOBOX_FILE:
        any =
          to_container<wxArrayString>(reinterpret_cast<wxComboBox*>(m_window))
            .get();
        break;

      case DIRPICKERCTRL:
        any = (reinterpret_cast<wxDirPickerCtrl*>(m_window))
                ->GetPath()
                .ToStdString();
        break;

      case FILEPICKERCTRL:
        any = (reinterpret_cast<wxFilePickerCtrl*>(m_window))
                ->GetPath()
                .ToStdString();
        break;

      case FONTPICKERCTRL:
        any =
          (reinterpret_cast<wxFontPickerCtrl*>(m_window))->GetSelectedFont();
        break;

      case GRID:
        any = ((wex::grid*)m_window)->get_cells_value();
        break;

      case HYPERLINKCTRL:
        any = (reinterpret_cast<wxHyperlinkCtrl*>(m_window))
                ->GetURL()
                .ToStdString();
        break;

      case LISTVIEW:
        any = ((wex::listview*)m_window)->save();
        break;

      case CHECKLISTBOX_BOOL:
      case RADIOBOX:
      case USER:
        // Not yet implemented
        break;

      case SLIDER:
        any = (reinterpret_cast<wxSlider*>(m_window))->GetValue();
        break;

      case SPINCTRL:
        any = (reinterpret_cast<wxSpinCtrl*>(m_window))->GetValue();
        break;

      case SPINCTRLDOUBLE:
        any = (reinterpret_cast<wxSpinCtrlDouble*>(m_window))->GetValue();
        break;

      case STATICBOX:
      case STATICLINE:
      case STATICTEXT:
        break;

      case TEXTCTRL:
        any =
          (reinterpret_cast<wxTextCtrl*>(m_window))->GetValue().ToStdString();
        break;

      case TEXTCTRL_FLOAT:
        if (const auto v = (reinterpret_cast<wxTextCtrl*>(m_window))
                             ->GetValue()
                             .ToStdString();
            !v.empty())
        {
          any = std::stod(v);
        }
        else
        {
          any = 0.0;
        }
        break;

      case TEXTCTRL_INT:
        if (const auto v = (reinterpret_cast<wxTextCtrl*>(m_window))
                             ->GetValue()
                             .ToStdString();
            !v.empty())
        {
          any = std::stol((reinterpret_cast<wxTextCtrl*>(m_window))
                            ->GetValue()
                            .ToStdString());
        }
        else
        {
          any = 0l;
        }

        break;

      case TOGGLEBUTTON:
        any = (reinterpret_cast<wxToggleButton*>(m_window))->GetValue();
        break;

      default:
        assert(0);
    }
  }
  catch (std::bad_cast& e)
  {
    wex::log(e) << *this << "get_value";
  }

  return any;
}

std::string wex::item::get_value_as_string() const
{
  switch (m_type)
  {
    case COMBOBOX:
    {
      auto ar(std::any_cast<wxArrayString>(get_value()));

      if (!ar.empty())
      {
        return ar.Item(0).ToStdString();
      }
    }
    break;

    case TEXTCTRL:
      return std::any_cast<std::string>(get_value());
      break;

    default:
      wex::log("not yet implemented for") << (int)m_type;
  }

  return std::string();
}

bool wex::item::is_notebook() const
{
  return m_type >= NOTEBOOK && m_type <= NOTEBOOK_WEX;
}

wxFlexGridSizer* wex::item::layout(
  wxWindow*        parent,
  wxSizer*         sizer,
  bool             readonly,
  wxFlexGridSizer* fgz)
{
  assert(sizer != nullptr);

  try
  {
    if (!create_window(parent, readonly))
    {
      return nullptr;
    }

    wxFlexGridSizer* return_sizer = nullptr;

    switch (m_type)
    {
      case COMBOBOX_DIR:
      case COMBOBOX_FILE:
        return_sizer = add_browse_button(sizer);
        break;

      case EMPTY:
        return fgz;

      case SPACER:
        sizer->AddSpacer(m_data.window().style());
        return fgz;

      case STATICBOX:
      {
        auto group(std::any_cast<group_t>(m_data.initial()));
        return_sizer = add(sizer, fgz);
        add_items(group, readonly);
      }
      break;

      default:
        if (is_notebook())
        {
          if (!m_data.initial().has_value())
          {
            log() << "invalid notebook";
            return nullptr;
          }

          auto* bookctrl = reinterpret_cast<wxBookCtrlBase*>(m_window);
          bookctrl->SetName("book-" + m_label_window);

          return_sizer = add(sizer, fgz);

          // Add all pages and recursive layout the subitems.
          for (auto& page : std::any_cast<notebook_t>(m_data.initial()))
          {
            add_items(page, readonly);
          }

          if (!wxPersistenceManager::Get().RegisterAndRestore(bookctrl))
          {
            if (bookctrl->GetPageCount() > 0)
            {
              // nothing was restored, so choose the default page ourselves
              bookctrl->SetSelection(0);
            }
          }
        }
        else
        {
          return_sizer = add(sizer, fgz);
        }
    }

    to_config(false);

    return return_sizer;
  }
  catch (std::bad_cast& e)
  {
    wex::log(e) << *this << "layout";
  }
  catch (std::exception& e)
  {
    wex::log(e) << *this << "layout";
  }

  return nullptr;
}

std::stringstream wex::item::log() const
{
  std::stringstream ss;

  ss << "item::LABEL: " << m_label << " "
     << "TYPE: " << m_type << " " << str("VALUE: ", get_value())
     << str("INITIAL: ", m_data.initial()) << str("MIN: ", m_data.min())
     << str("MAX: ", m_data.max()) << str("INC: ", m_data.inc());

  return ss;
}

void wex::item::set_dialog(item_template_dialog<item>* dlg)
{
  m_dialog = dlg;
}

bool wex::item::set_value(const std::any& value) const
{
  if (m_window == nullptr || !value.has_value())
  {
    return false;
  }

  try
  {
    switch (m_type)
    {
      case CHECKBOX:
        (reinterpret_cast<wxCheckBox*>(m_window))
          ->SetValue(std::any_cast<bool>(value));
        break;

      case CHECKLISTBOX_BIT:
      {
        auto* clb  = reinterpret_cast<wxCheckListBox*>(m_window);
        int   item = 0;

        for (const auto& b : std::any_cast<choices_t>(m_data.initial()))
        {
          clb->Check(item, (std::any_cast<long>(value) & b.first) > 0);
          item++;
        }
      }
      break;

      case COLOURPICKERWIDGET:
        (reinterpret_cast<wxColourPickerWidget*>(m_window))
          ->SetColour(std::any_cast<wxColour>(value));
        break;

      case COMBOBOX:
        combobox_as(
          reinterpret_cast<wxComboBox*>(m_window),
          std::any_cast<wxArrayString>(value));
        break;

      case DIRPICKERCTRL:
        (reinterpret_cast<wxDirPickerCtrl*>(m_window))
          ->SetPath(std::any_cast<std::string>(value));
        break;

      case FILEPICKERCTRL:
        (reinterpret_cast<wxFilePickerCtrl*>(m_window))
          ->SetPath(std::any_cast<std::string>(value));
        break;

      case FONTPICKERCTRL:
        (reinterpret_cast<wxFontPickerCtrl*>(m_window))
          ->SetSelectedFont(std::any_cast<wxFont>(value));
        break;

      case GRID:
      {
        auto* win = reinterpret_cast<grid*>(m_window);
        win->set_cells_value({0, 0}, std::any_cast<std::string>(value));
      }
      break;

      case LISTVIEW:
      {
        auto* win = reinterpret_cast<listview*>(m_window);
        win->load(std::any_cast<config::strings_t>(value));
      }
      break;

      case SLIDER:
        (reinterpret_cast<wxSlider*>(m_window))
          ->SetValue(std::any_cast<int>(value));
        break;

      case SPINCTRL:
        (reinterpret_cast<wxSpinCtrl*>(m_window))
          ->SetValue(std::any_cast<int>(value));
        break;

      case SPINCTRLDOUBLE:
        (reinterpret_cast<wxSpinCtrlDouble*>(m_window))
          ->SetValue(std::any_cast<double>(value));
        break;

      case TEXTCTRL:
        (reinterpret_cast<wxTextCtrl*>(m_window))
          ->SetValue(std::any_cast<std::string>(value));
        break;

      case TEXTCTRL_FLOAT:
        (reinterpret_cast<wxTextCtrl*>(m_window))
          ->SetValue(std::to_string(std::any_cast<double>(value)));
        break;

      case TEXTCTRL_INT:
        (reinterpret_cast<wxTextCtrl*>(m_window))
          ->SetValue(std::to_string(std::any_cast<long>(value)));
        break;

      case TOGGLEBUTTON:
        (reinterpret_cast<wxToggleButton*>(m_window))
          ->SetValue(std::any_cast<bool>(value));
        break;

      default:
        return false;
    }
  }
  catch (std::bad_cast& e)
  {
    wex::log(e) << *this << "set_value";
    return false;
  }

  return true;
}

bool wex::item::validate() const
{
  if (m_data.validate() != nullptr)
  {
    return m_data.validate()(get_value_as_string());
  }

  if (m_data.is_regex())
  {
    try
    {
      std::regex r(get_value_as_string());
    }
    catch (std::exception&)
    {
      return false;
    }
  }

  return true;
}

bool wex::item::validate(const std::string& regex) const
{
  try
  {
    return std::regex_match(get_value_as_string(), std::regex(regex));
  }
  catch (std::exception e)
  {
    return false;
  }
}
