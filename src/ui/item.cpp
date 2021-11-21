////////////////////////////////////////////////////////////////////////////////
// Name:      item.cpp
// Purpose:   Implementation of wex::item class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/tocontainer.h>
#include <wex/common/util.h>
#include <wex/core/log.h>
#include <wex/ui/item-template-dialog.h>

#include <sstream>

#include "item.h"
#include "ui.h"

namespace wex
{
const std::any get_value_prim(const wex::item* item)
{
  switch (item->type())
  {
    case item::CHECKBOX:
      if (item->data().initial().has_value())
      {
        return std::any_cast<bool>(item->data().initial());
      }
      else
      {
        return false;
      }

    case item::CHECKLISTBOX_BIT:
    case item::RADIOBOX:
    {
      long value = 0;

      for (const auto& b :
           std::any_cast<wex::item::choices_t>(item->data().initial()))
      {
        if (b.second.find(",") != std::string::npos)
        {
          value |= b.first;
        }
      }

      return std::any(value);
    }

    default:
      return item->data().initial();
  }
}

template <typename T> std::any get_value_simple(wxWindow* window)
{
  return (reinterpret_cast<T>(window))->GetValue();
}

bool get_value_simple(wex::item::type_t t, wxWindow* window, std::any& any)
{
  switch (t)
  {
    case item::CHECKBOX:
      any = get_value_simple<wxCheckBox*>(window);
      break;

    case item::SLIDER:
      any = get_value_simple<wxSlider*>(window);
      break;

    case item::SPINCTRL:
      any = get_value_simple<wxSpinCtrl*>(window);
      break;

    case item::SPINCTRLDOUBLE:
      any = get_value_simple<wxSpinCtrlDouble*>(window);
      break;

    case item::TOGGLEBUTTON:
      any = get_value_simple<wxToggleButton*>(window);
      break;

    default:
      return false;
  }

  return true;
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

const item::type_t use_type(const std::string& label, item::type_t t)
{
  return label.find(':') != std::string::npos ? item::STATICTEXT : t;
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

const std::any wex::item::get_value() const
{
  if (m_window == nullptr)
  {
    return get_value_prim(this);
  }

  if (is_notebook())
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

        case LISTVIEW:
          any = ((wex::listview*)m_window)->save();
          break;

        case CHECKLISTBOX_BOOL:
        case RADIOBOX:
        case USER:
          // Not yet implemented
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
            any = std::stol(v);
          }
          else
          {
            any = 0l;
          }
          break;

        case BUTTON:
        case STATICBOX:
        case STATICLINE:
        case STATICTEXT:
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
    wex::log(e) << "layout" << *this;
  }
  catch (std::exception& e)
  {
    wex::log(e) << "layout" << *this;
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
  catch (std::exception& e)
  {
    return false;
  }
}
