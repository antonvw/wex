////////////////////////////////////////////////////////////////////////////////
// Name:      item.cpp
// Purpose:   Implementation of wex::item class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <sstream>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/button.h>
#include <wx/clrpicker.h> // for wxColourPickerWidget
#include <wx/commandlinkbutton.h>
#include <wx/checklst.h>
#include <wx/filepicker.h>
#include <wx/fontpicker.h>
#include <wx/hyperlink.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>
#include <wx/tglbtn.h>
#ifndef __WXMSW__
#include <wx/valnum.h>
#endif
#include <wx/valtext.h>
#include <wx/window.h>

#include <wx/aui/auibook.h>
#include <wx/bookctrl.h> 
#include <wx/choicebk.h>
#include <wx/imaglist.h>
#include <wx/listbook.h>
#include <wx/notebook.h>
#include <wx/persist/treebook.h>
#include <wx/simplebook.h>
#include <wx/toolbook.h>

#include <wex/item.h>
#include <wex/grid.h>
#include <wex/itemtpldlg.h>
#include <wex/listview.h>
#include <wex/log.h>
#include <wex/notebook.h>
#include <wex/stc.h>
#include <wex/tocontainer.h>
#include <wex/util.h>

namespace wex
{
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
}

wex::item::item(type_t type, 
  const std::string& label, 
  const std::any& value, 
  label_t label_t,
  int major_dimension, 
  const std::any& min, 
  const std::any& max, 
  const std::any& inc,
  wxWindow* window, 
  user_window_create_t create, 
  user_window_to_config_t config,
  wxImageList* imageList)
  : m_type(type)
  , m_label_type(label_t)
  , m_label(label)
  , m_initial(value)
  , m_min(min)
  , m_max(max)
  , m_inc(inc)
  , m_major_dimension(major_dimension)
  , m_user_window_create_t(create)
  , m_window(window)
  , m_sizer_flags(wxSizerFlags().Border().Left())
  , m_user_window_to_config_t(config)
  , m_image_list(imageList)
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
    case STC:
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
    
    default: ; // prevent warning
  }
}

wxFlexGridSizer* wex::item::add(wxSizer* sizer, wxFlexGridSizer* current) const
{
  assert(m_window != nullptr);

  if (current == nullptr)
  {
    current = new wxFlexGridSizer(m_label_type == LABEL_LEFT ? 2: 1);
    current->AddGrowableCol(current->GetCols() - 1); // make last col growable
    sizer->Add(current, wxSizerFlags().Expand());
  }

  if (m_label_type != LABEL_NONE)
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

void wex::item::add_items(
  std::pair<std::string, std::vector<item>> & page, bool readonly)
{
  wxFlexGridSizer* previous_item_sizer = nullptr;
  int previous_type = -1;
  m_page = page.first;
  int use_cols = 1;
  if (m_major_dimension != -1) use_cols = m_major_dimension;
  
  if (const size_t col = m_page.find(":"); col != std::string::npos)
  {
    use_cols = std::stoi(m_page.substr(col + 1));
    m_page = m_page.substr(0, col);
  }

  auto* bookctrl = (wxBookCtrlBase*)m_window;
  
  int imageId = wxWithImages::NO_IMAGE;

  if (m_image_list != nullptr)
  {
    if ((int)bookctrl->GetPageCount() < m_image_list->GetImageCount())
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
  
  auto* booksizer = new wxFlexGridSizer(use_cols);
  bookctrl->GetCurrentPage()->SetSizer(booksizer);
  
  for (int i = 0; i < use_cols; i++)
  {
    booksizer->AddGrowableCol(i);
  }
  
  for (auto& item: page.second)
  {
    // If this item has same type as previous type use previous sizer,
    // otherwise use no sizer (layout will create a new one).
    wxFlexGridSizer* current_item_sizer = (
      item.type() == previous_type ? previous_item_sizer: nullptr);
    
    item.set_dialog(m_dialog);
    item.set_imagelist(m_image_list);

    previous_item_sizer = item.layout(
      bookctrl->GetCurrentPage(),
      booksizer,
      readonly,
      current_item_sizer);
    
    previous_type = item.type();
    
    if (m_dialog != nullptr)
    {
      m_dialog->add(item);
      m_dialog->bind_button(item);
    }

    if (booksizer->GetEffectiveRowsCount() >= 1 &&
       !booksizer->IsRowGrowable(booksizer->GetEffectiveRowsCount() - 1) &&
        item.is_row_growable())
    {
      booksizer->AddGrowableRow(booksizer->GetEffectiveRowsCount() - 1);
    }
  }
}
        
wxFlexGridSizer* wex::item::add_static_text(wxSizer* sizer) const
{
  assert(!m_label.empty());
  assert(m_window != nullptr);

  sizer->Add(
    new wxStaticText(m_window->GetParent(), 
      wxID_ANY, 
      m_label + ":"), 
      wxSizerFlags().Right().Border().Align(wxALIGN_LEFT));
  
  return nullptr;
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
      m_window = new wxButton(parent, 
        m_data.window().id(), 
        "default", 
        m_data.window().pos(), 
        m_data.window().size(), 
        m_data.window().style());
      ((wxButton *)m_window)->SetLabelMarkup(m_label);
      break;

    case CHECKBOX:
      m_window = new wxCheckBox(parent, 
        m_data.window().id(), 
        m_label, 
        m_data.window().pos(), 
        m_data.window().size(), 
        m_data.window().style());
      break;

    case CHECKLISTBOX_BIT:
      {
      wxArrayString arraychoices;

      for (const auto& it : std::any_cast<item::choices_t>(m_initial))
      {
        arraychoices.Add(it.second);
      }

      m_window = new wxCheckListBox(parent,
        m_data.window().id(), 
        m_data.window().pos(), 
        m_data.window().size(), 
        arraychoices, 
        m_data.window().style());
      }
      break;

    case CHECKLISTBOX_BOOL:
      {
      wxArrayString arraychoices;
      const std::set<std::string> & choices(std::any_cast<std::set<std::string>>(m_initial));
      arraychoices.resize(choices.size()); // required!
      std::copy (choices.begin(), choices.end(), arraychoices.begin());
      m_window = new wxCheckListBox(parent,
        m_data.window().id(), 
        m_data.window().pos(), 
        m_data.window().size(), 
        arraychoices, 
        m_data.window().style());
      }
      break;

    case COLOURPICKERWIDGET:
      m_window = new wxColourPickerWidget(parent, 
        m_data.window().id(), 
        *wxBLACK, 
        m_data.window().pos(), 
        m_data.window().size(), 
        m_data.window().style() == DATA_NUMBER_NOT_SET ? 
          wxCLRBTN_DEFAULT_STYLE: 
          m_data.window().style());
      break;

    case COMBOBOX:
    case COMBOBOX_DIR:
    case COMBOBOX_FILE:
      m_window = new wxComboBox(parent, 
        m_data.window().id(), 
        wxEmptyString,
        m_data.window().pos(), 
        m_data.window().size(),
        !m_initial.has_value() ? wxArrayString(): std::any_cast<wxArrayString>(m_initial),
        m_data.window().style());
      if (m_data.validator() != nullptr)
        m_window->SetValidator(*m_data.validator());
      break;

    case COMMANDLINKBUTTON:
      m_window = new wxCommandLinkButton(parent, 
        m_data.window().id(), 
        before(m_label, '\t'), 
        after(m_label, '\t'),
        m_data.window().pos(), 
        m_data.window().size(), 
        m_data.window().style());
      break;

    case DIRPICKERCTRL:
      {
      auto* pc = new wxDirPickerCtrl(parent, 
        m_data.window().id(), 
        !m_initial.has_value() ? std::string(): std::any_cast<std::string>(m_initial),
        wxDirSelectorPromptStr, 
        m_data.window().pos(), 
        m_data.window().size(), 
        m_data.window().style() == DATA_NUMBER_NOT_SET ? 
           wxDIRP_DEFAULT_STYLE: 
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

      auto* pc = new wxFilePickerCtrl(parent, 
        m_data.window().id(), 
        !m_initial.has_value() ? std::string(): std::any_cast<std::string>(m_initial),
        wxFileSelectorPromptStr, 
        wc,
        m_data.window().pos(), 
        m_data.window().size(),
        m_data.window().style() == DATA_NUMBER_NOT_SET ? 
          wxFLP_DEFAULT_STYLE: 
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
      auto* pc = new wxFontPickerCtrl(parent, 
        m_data.window().id(), 
        wxNullFont,
        m_data.window().pos(), 
        m_data.window().size(),
        m_data.window().style() == DATA_NUMBER_NOT_SET ? 
          wxFNTP_FONTDESC_AS_LABEL: 
          m_data.window().style());

      m_window = pc;

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
      m_window = new wxHyperlinkCtrl(parent, 
        m_data.window().id(), 
        m_label,
        std::any_cast<std::string>(m_initial), 
        m_data.window().pos(), 
        m_data.window().size(), 
        m_data.window().style() == DATA_NUMBER_NOT_SET ? 
          wxHL_DEFAULT_STYLE: 
          m_data.window().style());
      break;

    case LISTVIEW:
      {
      auto* lv = new listview(m_listview_data.
        window(window_data(m_data.window()).parent(parent)));
      lv->item_from_text(
        !m_initial.has_value() ? 
           std::string(): 
           std::any_cast<std::string>(m_initial));
      m_window = lv;
      }
      break;

    case NOTEBOOK: 
      bookctrl = new wxNotebook(parent, 
        m_data.window().id(), 
        m_data.window().pos(), 
        m_data.window().size(), 
        m_data.window().style());
      break;

    case NOTEBOOK_AUI: 
      bookctrl = new wxAuiNotebook(parent,
        m_data.window().id(), 
        m_data.window().pos(), 
        m_data.window().size(), 
        m_data.window().style() == DATA_NUMBER_NOT_SET ? 
          wxAUI_NB_DEFAULT_STYLE: 
          m_data.window().style());
      break;

    case NOTEBOOK_CHOICE: 
      bookctrl = new wxChoicebook(parent, 
        m_data.window().id(), 
        m_data.window().pos(), 
        m_data.window().size(), 
        m_data.window().style());
      break;

    case NOTEBOOK_LIST: 
      bookctrl = new wxListbook(parent, 
        m_data.window().id(), 
        m_data.window().pos(), 
        m_data.window().size(), 
        m_data.window().style());
      break;

    case NOTEBOOK_SIMPLE: 
      bookctrl = new wxSimplebook(parent, 
        m_data.window().id(), 
        m_data.window().pos(), 
        m_data.window().size(), 
        m_data.window().style());
      break;
          
    case NOTEBOOK_TOOL: 
      bookctrl = new wxToolbook(parent, 
        m_data.window().id(), 
        m_data.window().pos(), 
        m_data.window().size(), 
        m_data.window().style());

      if (m_image_list == nullptr)
      {
        log() << "toolbook requires image list";
        return false;
      }
      break;
    
    case NOTEBOOK_TREE: 
      bookctrl = new wxTreebook(parent, 
        m_data.window().id(), 
        m_data.window().pos(), 
        m_data.window().size(), 
        m_data.window().style());
      break;
    
    case NOTEBOOK_WEX: 
      bookctrl = new notebook(
        m_data.window(window_data().parent(parent)).window()); 
      break;

    case RADIOBOX:
      {
      wxArrayString arraychoices;

      for (const auto& it : std::any_cast<item::choices_t>(m_initial))
      {
        arraychoices.Add(it.second);
      } 

      m_window = new wxRadioBox(parent, 
        m_data.window().id(), 
        m_label, 
        m_data.window().pos(), 
        m_data.window().size(), 
        arraychoices, 
        m_major_dimension, 
        m_data.window().style());
      }
      break;

    case SLIDER:
      m_window = new wxSlider(parent, 
        m_data.window().id(), 
        std::any_cast<int>(m_initial),
        std::any_cast<int>(m_min), 
        std::any_cast<int>(m_max),
        m_data.window().pos(), 
        m_data.window().size(), 
        m_data.window().style());
      break;

    case SPINCTRL:
      m_window = new wxSpinCtrl(parent, 
        m_data.window().id(), 
        wxEmptyString,
        m_data.window().pos(), 
        m_data.window().size(),
        wxSP_ARROW_KEYS | (readonly ? wxTE_READONLY: 0),
        std::any_cast<int>(m_min), 
        std::any_cast<int>(m_max), 
        !m_initial.has_value() ? std::any_cast<int>(m_min): std::any_cast<int>(m_initial));
      break;

    case SPINCTRLDOUBLE:
      m_window = new wxSpinCtrlDouble(parent, 
        m_data.window().id(), 
        wxEmptyString,
        m_data.window().pos(), 
        m_data.window().size(),
        wxSP_ARROW_KEYS | (readonly ? wxTE_READONLY: 0),
        std::any_cast<double>(m_min), 
        std::any_cast<double>(m_max), 
        !m_initial.has_value() ? std::any_cast<double>(m_min): std::any_cast<double>(m_initial), 
        std::any_cast<double>(m_inc));
      break;

    case STATICLINE:
      m_window = new wxStaticLine(parent, 
        m_data.window().id(),
        m_data.window().pos(), 
        m_data.window().size(), 
        m_data.window().style());
      break;

    case STATICTEXT:
      m_window = new wxStaticText(parent, 
        m_data.window().id(), 
        wxEmptyString,
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style());
      ((wxStaticText* )m_window)->SetLabelMarkup(m_label);
      break;

    case STC:
      m_window = new stc(std::string(),
        stc_data().
          menu(stc_data::menu_t().set(
            stc_data::MENU_CONTEXT).set(
            stc_data::MENU_OPEN_LINK).set(
            stc_data::MENU_VCS)).
          window(window_data(m_data.window()).
            parent(parent)));
      
      // Do not use vi mode, as ESC should cancel the dialog,
      // and would not be interpreted by vi.
      ((stc* )m_window)->get_vi().use(false);

      if (m_initial.has_value())
      {
        ((stc* )m_window)->get_lexer().set(std::any_cast<std::string>(m_initial));
      }
      break;

    case TEXTCTRL:
      m_window = new wxTextCtrl(parent, 
        m_data.window().id(), 
        !m_initial.has_value() ? 
           std::string(): std::any_cast<std::string>(m_initial),
        m_data.window().pos(),
        m_data.window().size(),
        m_data.window().style() | (readonly ? wxTE_READONLY: 0));
      if (m_data.validator() != nullptr)
        m_window->SetValidator(*m_data.validator());
      break;

    case TEXTCTRL_FLOAT:
      m_window = new wxTextCtrl(parent, 
        m_data.window().id(), 
        !m_initial.has_value() ? 
           std::string(): std::any_cast<std::string>(m_initial),
        m_data.window().pos(), 
        m_data.window().size(), 
        m_data.window().style() | (readonly ? wxTE_READONLY: 0) | wxTE_RIGHT);
      if (m_data.validator() == nullptr)
#ifdef __WXMSW__
        // TODO: using wxFloatingPointValidator and wxIntegerValidator
        // gives compile error in MSW.
        ;
#else
        m_window->SetValidator(wxFloatingPointValidator<double>());
#endif
      else 
        m_window->SetValidator(*m_data.validator());
      break;
      
    case TEXTCTRL_INT:
      m_window = new wxTextCtrl(parent, 
        m_data.window().id(), 
        !m_initial.has_value() ? 
           std::string(): std::any_cast<std::string>(m_initial),
        m_data.window().pos(), 
        m_data.window().size(), 
        m_data.window().style() | (readonly ? wxTE_READONLY: 0) | wxTE_RIGHT);
      if (m_data.validator() == nullptr)
#ifdef __WXMSW__
        ;
#else
        m_window->SetValidator(wxIntegerValidator<int>());
#endif
      else 
        m_window->SetValidator(*m_data.validator());
      break;

    case TOGGLEBUTTON:
      m_window = new wxToggleButton(parent, 
        m_data.window().id(), 
        m_label,
        m_data.window().pos(), 
        m_data.window().size(), 
        m_data.window().style());
      break;

    case USER:
      if (m_user_window_create_t != nullptr)
      {
        assert(m_window != nullptr);
        (m_user_window_create_t)(m_window, parent, readonly);
      }
      break;
  
    default: assert(0);
  }

  if (bookctrl != nullptr)
  {
    m_window = bookctrl;

    if (m_image_list != nullptr)
    {
      bookctrl->SetImageList(m_image_list);
    }
  }
  
  if (m_type != EMPTY && m_type != SPACER)
  {
    assert(m_window != nullptr);
  }

  return true;
}

const std::any wex::item::get_value() const
{
  if (m_window == nullptr) return std::any();
  
  std::any any;

  try
  {  
    switch (m_type)
    {
      case CHECKBOX:
        any = ((wxCheckBox* )m_window)->GetValue(); 
        break;
      
      case CHECKLISTBOX_BIT: {
        auto* clb = (wxCheckListBox*)window();
        long value = 0;
        int item = 0;

        for (const auto& b : std::any_cast<item::choices_t>(m_initial))
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
        any = ((wxColourPickerWidget* )m_window)->GetColour(); 
        break;
      
      case COMBOBOX: 
      case COMBOBOX_DIR: 
      case COMBOBOX_FILE: 
        any = to_container<wxArrayString>((wxComboBox*)m_window).get(); 
        break;
      
      case DIRPICKERCTRL: 
        any = ((wxDirPickerCtrl* )m_window)->GetPath().ToStdString(); 
        break;
      
      case FILEPICKERCTRL: 
        any = ((wxFilePickerCtrl* )m_window)->GetPath().ToStdString(); 
        break;
      
      case FONTPICKERCTRL: 
        any = ((wxFontPickerCtrl* )m_window)->GetSelectedFont(); 
        break;
      
      case GRID: 
        any = ((wex::grid* )m_window)->get_cells_value(); 
        break;

      case HYPERLINKCTRL: 
        any = ((wxHyperlinkCtrl* )m_window)->GetURL().ToStdString();
        break;
      
      case LISTVIEW: 
        any = ((wex::listview* )m_window)->item_to_text(-1); 
        break;
      
      case SLIDER: 
        any = ((wxSlider* )m_window)->GetValue(); 
        break;
      
      case SPINCTRL: 
        any = ((wxSpinCtrl* )m_window)->GetValue(); 
        break;
      
      case SPINCTRLDOUBLE: 
        any = ((wxSpinCtrlDouble* )m_window)->GetValue(); 
        break;
      
      case STC: 
        any = ((wxStyledTextCtrl* )m_window)->GetValue().ToStdString(); 
        break;
      
      case TEXTCTRL: 
        any = ((wxTextCtrl* )m_window)->GetValue().ToStdString();  
        break;

      case TEXTCTRL_FLOAT: 
        if (const auto v = ((wxTextCtrl* )m_window)->GetValue().ToStdString();
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
        if (const auto v = ((wxTextCtrl* )m_window)->GetValue().ToStdString(); 
          !v.empty())
        {
          any = std::stol(((wxTextCtrl* )m_window)->GetValue().ToStdString()); 
        }
        else
        {
          any = 0l;
        } 
  
        break;
      
      case TOGGLEBUTTON: 
        any = ((wxToggleButton* )m_window)->GetValue(); 
        break;
      
      default: ; // prevent warning
    }
  }
  catch (std::bad_cast& e)
  {
    wex::log(e) << *this << "get_value";
  }

  return any;
}
  
wxFlexGridSizer* wex::item::layout(
  wxWindow* parent, 
  wxSizer* sizer, 
  bool readonly,
  wxFlexGridSizer* fgz)
{
  assert(sizer != nullptr);

  try
  {  
    if (!create_window(parent, readonly))
    {
      return nullptr;
    }
    
    wxFlexGridSizer* return_sizer;
    
    switch (m_type)
    {
      case COMBOBOX_DIR: 
      case COMBOBOX_FILE: 
        return_sizer = add_browse_button(sizer);
        break;

      case EMPTY: 
        return fgz;

      case SPACER: sizer->AddSpacer(m_data.window().style()); 
        return fgz;
      
      default: 
        if (m_type >= NOTEBOOK && m_type <= NOTEBOOK_WEX)
        {
          if (!m_initial.has_value())
          {
            log() << "illegal notebook";
            return nullptr;
          }
          
          auto* bookctrl = (wxBookCtrlBase*)m_window;
          bookctrl->SetName("book-" + m_label);
          
          return_sizer = add(sizer, fgz);
          
          // Add all pages and recursive layout the subitems.
          for (auto& page : std::any_cast<items_notebook_t>(m_initial))
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
  
  return nullptr;
}

std::stringstream wex::item::log() const
{
  std::stringstream ss;

  ss << "LABEL: " << m_label << " " 
     << "TYPE: " << m_type << " "
     << str("VALUE: ", get_value())
     << str("INITIAL: ", m_initial)
     << str("MIN: ", m_min)
     << str("MAX: ", m_max)
     << str("INC: ", m_inc);

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
        ((wxCheckBox* )m_window)->SetValue(std::any_cast<bool>(value)); 
        break;
      
      case CHECKLISTBOX_BIT:
        {
        auto* clb = (wxCheckListBox*)m_window;
        int item = 0;

        for (const auto& b : std::any_cast<item::choices_t>(m_initial))
        {
          clb->Check(item, (std::any_cast<long>(value) & b.first) > 0);
          item++;
        }
        }
        break;
      
      case COLOURPICKERWIDGET: 
        ((wxColourPickerWidget* )m_window)->
          SetColour(std::any_cast<wxColour>(value)); 
        break;
      
      case COMBOBOX: 
        combobox_as((wxComboBox* )m_window, std::any_cast<wxArrayString>(value)); 
        break;
      
      case DIRPICKERCTRL: 
        ((wxDirPickerCtrl* )m_window)->
          SetPath(std::any_cast<std::string>(value)); 
        break;
      
      case FILEPICKERCTRL: 
        ((wxFilePickerCtrl* )m_window)->
          SetPath(std::any_cast<std::string>(value)); 
        break;
      
      case FONTPICKERCTRL: 
        ((wxFontPickerCtrl* )m_window)->
          SetSelectedFont(std::any_cast<wxFont>(value)); 
        break;
      
      case GRID: 
        {
        auto* win = (grid*)window();
        win->set_cells_value({0, 0}, std::any_cast<std::string>(value));
        }
        break;

      case LISTVIEW:
        {
        auto* win = (listview*)window();
        win->clear();
        win->item_from_text(std::any_cast<std::string>(value));
        }
        break;

      case SLIDER: 
        ((wxSlider* )m_window)->
          SetValue(std::any_cast<int>(value)); 
        break;
      
      case SPINCTRL: 
        ((wxSpinCtrl* )m_window)->
          SetValue(std::any_cast<int>(value)); 
        break;
      
      case SPINCTRLDOUBLE: 
        ((wxSpinCtrlDouble* )m_window)->
          SetValue(std::any_cast<double>(value)); 
        break;
      
      case STC: 
        ((wxStyledTextCtrl* )m_window)->
          SetValue(std::any_cast<std::string>(value)); 
        break;
      
      case TEXTCTRL: 
        ((wxTextCtrl* )m_window)->
          SetValue(std::any_cast<std::string>(value)); 
        break;
      
      case TEXTCTRL_FLOAT: 
        ((wxTextCtrl* )m_window)->
          SetValue(std::to_string(std::any_cast<double>(value))); 
        break;
      
      case TEXTCTRL_INT: 
        ((wxTextCtrl* )m_window)->
          SetValue(std::to_string(std::any_cast<long>(value))); 
        break;
      
      case TOGGLEBUTTON: 
        ((wxToggleButton* )m_window)->
          SetValue(std::any_cast<bool>(value)); 
        break;

      default: return false;
    }
  }
  catch (std::bad_cast& e)
  {
    wex::log(e) << *this << "set_value";
    return false;
  }
  
  return true;
}
