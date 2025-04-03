////////////////////////////////////////////////////////////////////////////////
// Name:      item.cpp
// Purpose:   Implementation of wex::item class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <charconv>
#include <sstream>

#include <wex/common/tocontainer.h>
#include <wex/common/util.h>
#include <wex/core/log.h>
#include <wex/ui/item-template-dialog.h>

#include "item.h"
#include "ui.h"

#define DO_DIALOG                                                              \
  {                                                                            \
    /* NOLINTNEXTLINE */                                                       \
    if (dlg.ShowModal() == wxID_OK)                                            \
    {                                                                          \
      const auto value = dlg.GetPath();                                        \
      const int  item  = cb->FindString(value);                                \
      cb->SetSelection(item == wxNOT_FOUND ? cb->Append(value) : item);        \
    }                                                                          \
  }

wex::item::item(
  type_t             type,
  const std::string& label,
  const std::any&    value,
  const data::item&  data)
  : m_type(type)
  , m_data(data)
  , m_label(label)
  , m_label_window(rfind_after(label, "."))
  , m_sizer_flags(
      m_type == GROUP ? wxSizerFlags().Left() : wxSizerFlags().Border().Left())
  , m_reflect(
      {REFLECT_ADD("label", m_label),
       REFLECT_ADD("type", std::to_string(m_type)),
       REFLECT_ADD("value", get_value()),
       REFLECT_ADD("initial", m_data.initial()),
       REFLECT_ADD("min", m_data.min()),
       REFLECT_ADD("max", m_data.max()),
       REFLECT_ADD("inc", m_data.inc())})
{
  m_data.initial(value);

  if (is_notebook())
  {
    m_is_row_growable = true;
    m_sizer_flags.Expand();
  }
  else
  {
    switch (m_type)
    {
      case CHECKLISTBOX_BIT:
      case CHECKLISTBOX_BOOL:
      case GRID:
      case LISTVIEW:
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
      case GROUP:
      case LISTBOX:
      case SPACER:
      case STATICLINE:
      case USER:
        m_sizer_flags.Expand();
        break;

      default:; // prevent warning
    }
  }
}

wex::item::item(int size)
  : item(SPACER)
{
  m_data.window(data::window().style(size));
}

wex::item::item(wxOrientation orientation)
  : item(STATICLINE)
{
  m_data.window(data::window().style(orientation));
}

wex::item::item(
  const std::string& label,
  int                min,
  int                max,
  const std::any&    value,
  type_t             type,
  const data::item&  data)
  : item(
      type,
      label,
      value,
      data::item(data)
        .window(data::window().style(wxSP_ARROW_KEYS))
        .min(min)
        .max(max))
{
}

wex::item::item(
  const std::string& label,
  double             min,
  double             max,
  const std::any&    value,
  const data::item&  data)
  : item(SPINCTRLDOUBLE, label, value, data::item(data).min(min).max(max))
{
}

wex::item::item(const choices_bool_t& choices, const data::item& data)
  : item(CHECKLISTBOX_BOOL, "checklistbox_bool", choices, data)
{
}

wex::item::item(
  const std::string& label,
  const notebook_t&  v,
  type_t             type,
  const data::item&  data)
  : item(type, label, v, data)
{
}

wex::item::item(const group_t& g, const data::item& data)
  : item(g.first.empty() ? GROUP : STATICBOX, g.first, g, data)
{
}

wex::item::item(
  const std::string& label,
  const choices_t&   choices,
  bool               use_radiobox,
  const data::item&  data)
  : item(
      use_radiobox ? RADIOBOX : CHECKLISTBOX_BIT,
      label,
      choices,
      data::item(data)
        .window(data::window().style(wxRA_SPECIFY_COLS))
        .label_type(data::item::LABEL_NONE))
{
}

wex::item::item(
  const std::string& label,
  wxWindow*          window,
  const data::item&  data)
  : item(USER, label, std::string(), data)
{
  m_window = window;
}

wex::item::item(
  const std::string&    label,
  const data::listview& data,
  const std::any&       value,
  const data::item&     d)
  : item(LISTVIEW, label, value, d)
{
  m_data_listview = data;
}

wex::item::item(
  const std::string& label,
  type_t             type,
  const std::any&    value,
  const data::item&  data)
  : item(
      type,
      label,
      value,
      data::item(data).label_type(
        type == BUTTON || type == CHECKBOX || type == COMMANDLINKBUTTON ||
            type == TOGGLEBUTTON ?
          data::item::LABEL_NONE :
          data.label_type()))
{
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

wex::data::layout::sizer_t* wex::item::add(data::layout& layout) const
{
  if (layout.sizer_layout() == nullptr)
  {
    layout.sizer_layout_create(new wex::data::layout::sizer_t(
      m_data.label_type() == data::item::LABEL_LEFT ? 2 : 1));
    layout.sizer_layout_grow_col();
    layout.sizer()->Add(layout.sizer_layout(), wxSizerFlags().Expand());
  }

  if (m_data.label_type() != data::item::LABEL_NONE && !m_label.empty())
  {
    add_static_text(layout.sizer_layout());
  }

  if (m_window != nullptr)
  {
    layout.sizer_layout()->Add(m_window, m_sizer_flags);
  }

  if (m_is_row_growable)
  {
    layout.sizer_layout_grow_row();
  }

  return layout.sizer_layout();
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
  auto* browse = new wxButton(
    m_window->GetParent(),
    m_window->GetId(),
    // see generic/filepickerg.cpp and wxPB_SMALL,
    // as we use wxFLP_SMALL do the same as there
    _("..."),
    wxDefaultPosition,
    wxDefaultSize,
    wxBU_EXACTFIT);

  fgz->Add(browse, wxSizerFlags().Center());

  sizer->Add(fgz, wxSizerFlags().Left().Expand()); // no border

  switch (m_type)
  {
    case COMBOBOX_DIR:
      browse->Bind(
        wxEVT_BUTTON,
        [&, window = this->m_window, label = this->m_label](
          const wxCommandEvent& event)
        {
          if (auto* cb = reinterpret_cast<wxComboBox*>(window); cb != nullptr)
          {
            wxDirDialog dlg(
              window,
              wxDirSelectorPromptStr,
              cb->GetValue(),
              wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
            DO_DIALOG;
          }
          else
          {
            wex::log("browse dir failed") << label;
          }
        },
        m_window->GetId());
      break;

    case COMBOBOX_FILE:
      browse->Bind(
        wxEVT_BUTTON,
        [&, window = this->m_window, label = this->m_label](
          const wxCommandEvent& event)
        {
          if (auto* cb = reinterpret_cast<wxComboBox*>(window); cb != nullptr)
          {
            const path   path(cb->GetValue());
            wxFileDialog dlg(
              window,
              wxFileSelectorPromptStr,
              path.parent_path(),
              path.filename(),
              wxFileSelectorDefaultWildcardStr,
              wxFD_DEFAULT_STYLE | wxFD_FILE_MUST_EXIST);
            DO_DIALOG;
          }
          else
          {
            wex::log("browse file failed") << label;
          }
        },
        window()->GetId());
      break;

    default:
      assert(0);
  }

  return fgz;
}

void wex::item::add_items(group_t& page, bool readonly)
{
  int use_cols = (m_data.columns() != -1 ? m_data.columns() : 1);

  if (auto* bookctrl = dynamic_cast<wxBookCtrlBase*>(m_window);
      bookctrl != nullptr)
  {
    m_page      = page.first;
    int imageId = wxWithImages::NO_IMAGE;

    if (const auto col = m_page.find(':'); col != std::string::npos)
    {
      std::from_chars(
        m_page.data() + col + 1,
        m_page.data() + m_page.size(),
        use_cols);

      log::trace("found page cols") << use_cols;

      m_page = m_page.substr(0, col);
    }

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

    data::layout layout(bookctrl->GetCurrentPage(), use_cols);
    layout.is_readonly(readonly);

    bookctrl->GetCurrentPage()->SetSizer(layout.sizer());

    for (int i = 0; i < use_cols; i++)
    {
      layout.sizer()->AddGrowableCol(i);
    }

    add_items(layout, page.second);
  }
  else if (auto* sb = dynamic_cast<wxStaticBox*>(m_window); sb != nullptr)
  {
    data::layout layout(sb, use_cols);
    layout.is_readonly(readonly);
    add_items(layout, page.second);
    sb->SetSizerAndFit(layout.sizer());
  }
  else if (m_type == GROUP)
  {
    auto*        panel = dynamic_cast<wxPanel*>(m_window);
    data::layout layout(panel, page.second.size());
    layout.sizer()->AddGrowableCol(0);
    layout.is_readonly(readonly);
    add_items(layout, page.second);
    panel->SetSizerAndFit(layout.sizer());
  }
  else
  {
    assert(0);
  }
}

void wex::item::add_items(data::layout& layout, std::vector<item>& v)
{
  wex::data::layout::sizer_t* previous_item_sizer = nullptr;

  for (int previous_type = -1; auto& item : v)
  {
    // If this item has the same type as previous type use previous sizer,
    // otherwise use no sizer (layout will create a new one)
    // (see also item_template_dialog::layout).
    layout.sizer_layout_create(
      item.type() == previous_type ? previous_item_sizer : nullptr);

    previous_item_sizer = item.layout(layout);

    previous_type = item.type();

    if (m_dialog != nullptr)
    {
      item.set_dialog(m_dialog);
      m_dialog->add(item);
      m_dialog->bind_button(item);
    }

    if (item.is_row_growable())
    {
      layout.sizer_grow_row();
    }
  }
}

void wex::item::add_static_text(wxSizer* sizer) const
{
  if (!m_label_window.empty() && m_window != nullptr)
  {
    sizer->Add(
      new wxStaticText(m_window->GetParent(), wxID_ANY, m_label_window + ":"),
      wxSizerFlags().Right().Border().Align(wxALIGN_LEFT));
  }
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

const std::any wex::item::get_value() const
{
  if (m_window == nullptr)
  {
    return get_value_prim(this);
  }

  if (is_notebook() || no_value(m_type))
  {
    return std::any();
  }

  std::any any;

  try
  {
    if (!get_value_simple(m_type, m_window, any))
    {
      switch (m_type)
      {
        case CHECKLISTBOX_BIT:
        {
          auto* clb   = reinterpret_cast<wxCheckListBox*>(m_window);
          long  value = 0;

          for (int         item = 0;
               const auto& b : std::any_cast<choices_t>(m_data.initial()))
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
          any =
            (reinterpret_cast<wxColourPickerWidget*>(m_window))->GetColour();
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

        case LISTBOX:
          any = listbox_to_list(reinterpret_cast<wxListBox*>(m_window));
          break;

        case LISTVIEW:
          any = ((wex::listview*)m_window)->save();
          break;

        case TEXTCTRL:
          any =
            (reinterpret_cast<wxTextCtrl*>(m_window))->GetValue().ToStdString();
          break;

        case TEXTCTRL_FLOAT:
          if (const auto& v = (reinterpret_cast<wxTextCtrl*>(m_window))
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
          if (const auto& v = (reinterpret_cast<wxTextCtrl*>(m_window))
                                ->GetValue()
                                .ToStdString();
              !v.empty())
          {
            long val = 0; // value if from_chars has ec
            std::from_chars(v.data(), v.data() + v.size(), val);
            any = val;
          }
          else
          {
            any = 0L;
          }
          break;

        default:
          assert(0);
      }
    }
  }
  catch (std::bad_cast& e)
  {
    wex::log(e) << "get_value" << *this;
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

wex::data::layout::sizer_t* wex::item::layout(data::layout& layout)
{
  assert(layout.sizer() != nullptr);

  try
  {
    if (!create_window(layout.parent(), layout.is_readonly()))
    {
      return nullptr;
    }

    wex::data::layout::sizer_t* return_sizer = nullptr;

    switch (m_type)
    {
      case COMBOBOX_DIR:
      case COMBOBOX_FILE:
        return_sizer = add_browse_button(layout.sizer());
        break;

      case EMPTY:
        return layout.sizer_layout();

      case SPACER:
        layout.sizer()->AddSpacer(m_data.window().style());
        return layout.sizer_layout();

      case GROUP:
      case STATICBOX:
      {
        auto group(std::any_cast<group_t>(m_data.initial()));
        return_sizer = add(layout);
        add_items(group, layout.is_readonly());
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

          return_sizer = add(layout);

          // Add all pages and recursive layout the subitems.
          for (auto& page : std::any_cast<notebook_t>(m_data.initial()))
          {
            add_items(page, layout.is_readonly());
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
          return_sizer = add(layout);
        }
    }

    to_config(false);

    return return_sizer;
  }
  catch (std::bad_cast& e)
  {
    wex::log(e) << "item::layout" << *this;
  }
  catch (std::exception& e)
  {
    wex::log(e) << "item::layout" << *this;
  }

  return nullptr;
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
        auto* clb = reinterpret_cast<wxCheckListBox*>(m_window);

        for (int         item = 0;
             const auto& b : std::any_cast<choices_t>(m_data.initial()))
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
        (reinterpret_cast<grid*>(m_window))
          ->set_cells_value({0, 0}, std::any_cast<std::string>(value));
      }
      break;

      case LISTBOX:
        listbox_as(
          reinterpret_cast<wxListBox*>(m_window),
          std::any_cast<strings_t>(value));
        break;

      case LISTVIEW:
        (reinterpret_cast<listview*>(m_window))
          ->load(std::any_cast<strings_t>(value));
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
    wex::log(e) << "set_value" << *this;
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
      boost::regex r(get_value_as_string());
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
    return boost::regex_match(get_value_as_string(), boost::regex(regex));
  }
  catch (const std::exception& e)
  {
    return false;
  }
}
