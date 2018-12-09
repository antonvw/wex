////////////////////////////////////////////////////////////////////////////////
// Name:      item.cpp
// Purpose:   Implementation of wex::item class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
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
#include <wex/itemtpldlg.h>
#include <wex/listview.h>
#include <wex/log.h>
#include <wex/notebook.h>
#include <wex/stc.h>
#include <wex/tocontainer.h>
#include <wex/util.h>

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
  : m_Type(type)
  , m_LabelType(label_t)
  , m_Label(label)
  , m_Initial(value)
  , m_Min(min)
  , m_Max(max)
  , m_Inc(inc)
  , m_MajorDimension(major_dimension)
  , m_user_window_create_t(create)
  , m_Window(window)
  , m_SizerFlags(wxSizerFlags().Border().Left())
  , m_user_window_to_config_t(config)
  , m_ImageList(imageList)
{
  switch (m_Type)
  {
    case CHECKLISTBOX_BIT:
    case CHECKLISTBOX_BOOL:
    case LISTVIEW:
    case NOTEBOOK: 
    case NOTEBOOK_AUI: 
    case NOTEBOOK_EX: 
    case NOTEBOOK_LIST: 
    case NOTEBOOK_SIMPLE: 
    case NOTEBOOK_TOOL: 
    case NOTEBOOK_TREE: 
    case RADIOBOX:
    case STC:
      m_is_row_growable = true;
      m_SizerFlags.Expand();
      break;

    case STATICTEXT:
    case TEXTCTRL:
    case TEXTCTRL_FLOAT:
    case TEXTCTRL_INT:
      m_is_row_growable = (m_Data.window().style() & wxTE_MULTILINE) > 0;
      m_SizerFlags.Expand();
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
      m_SizerFlags.Expand();
      break;
    
    default: ; // prevent warning
  }
}

wxFlexGridSizer* wex::item::Add(wxSizer* sizer, wxFlexGridSizer* current) const
{
  assert(m_Window != nullptr);

  if (current == nullptr)
  {
    current = new wxFlexGridSizer(m_LabelType == LABEL_LEFT ? 2: 1);
    current->AddGrowableCol(current->GetCols() - 1); // make last col growable
    sizer->Add(current, wxSizerFlags().Expand());
  }

  if (m_LabelType != LABEL_NONE)
  {
    AddStaticText(current);
  }
  
  current->Add(m_Window, m_SizerFlags);
  
  if (m_is_row_growable && current->GetEffectiveRowsCount() >= 1)
  {
    current->AddGrowableRow(current->GetEffectiveRowsCount() - 1);
  }
  
  return current;
}
      
wxFlexGridSizer* wex::item::AddBrowseButton(wxSizer* sizer)
{
  assert(m_Window != nullptr);

  wxFlexGridSizer* fgz = new wxFlexGridSizer(3);
  fgz->AddGrowableCol(1);

  AddStaticText(fgz);

  fgz->Add(m_Window, m_SizerFlags);

  // Tried to use a wxDirPickerCtrl here, is nice,
  // but does not use a combobox for keeping old values, so this is better.
  // And the text box that is used is not resizable as well.
  // In item_template_dialog the button click is bound to browse dialog.
  wxButton* browse = new wxButton(
    m_Window->GetParent(),
    m_Window->GetId(),
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
  m_Page = page.first;
  int use_cols = 1;
  if (m_MajorDimension != -1) use_cols = m_MajorDimension;
  
  if (const size_t col = m_Page.find(":"); col != std::string::npos)
  {
    use_cols = std::stoi(m_Page.substr(col + 1));
    m_Page = m_Page.substr(0, col);
  }

  wxBookCtrlBase* bookctrl = (wxBookCtrlBase*)m_Window;
  
  int imageId = wxWithImages::NO_IMAGE;

  if (m_ImageList != nullptr)
  {
    if ((int)bookctrl->GetPageCount() < m_ImageList->GetImageCount())
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
    m_Page,
    true, // select
    imageId);
  
  wxFlexGridSizer* booksizer = new wxFlexGridSizer(use_cols);
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
    
    item.set_dialog(m_Dialog);
    item.set_imagelist(m_ImageList);

    previous_item_sizer = item.layout(
      bookctrl->GetCurrentPage(),
      booksizer,
      readonly,
      current_item_sizer);
    
    previous_type = item.type();
    
    if (m_Dialog != nullptr)
    {
      m_Dialog->add(item);
      m_Dialog->bind_button(item);
    }

    if (booksizer->GetEffectiveRowsCount() >= 1 &&
       !booksizer->IsRowGrowable(booksizer->GetEffectiveRowsCount() - 1) &&
        item.is_row_growable())
    {
      booksizer->AddGrowableRow(booksizer->GetEffectiveRowsCount() - 1);
    }
  }
}
        
wxFlexGridSizer* wex::item::AddStaticText(wxSizer* sizer) const
{
  assert(!m_Label.empty());
  assert(m_Window != nullptr);

  sizer->Add(
    new wxStaticText(m_Window->GetParent(), 
      wxID_ANY, 
      m_Label + ":"), 
      wxSizerFlags().Right().Border().Align(wxALIGN_LEFT));
  
  return nullptr;
}

bool wex::item::CreateWindow(wxWindow* parent, bool readonly)
{
  if (m_Type != USER && m_Window != nullptr)
  {
    return false;
  }
  
  wxBookCtrlBase* bookctrl = nullptr;

  switch (m_Type)
  {
    case EMPTY:
    case SPACER:
      break;
      
    case BUTTON:
      // Using a label is necessary for wxGTK.
      m_Window = new wxButton(parent, 
        m_Data.window().id(), 
        "default", 
        m_Data.window().pos(), 
        m_Data.window().size(), 
        m_Data.window().style());
      ((wxButton *)m_Window)->SetLabelMarkup(m_Label);
      break;

    case CHECKBOX:
      m_Window = new wxCheckBox(parent, 
        m_Data.window().id(), 
        m_Label, 
        m_Data.window().pos(), 
        m_Data.window().size(), 
        m_Data.window().style());
      break;

    case CHECKLISTBOX_BIT:
      {
      wxArrayString arraychoices;

      for (const auto& it : std::any_cast<item::choices_t>(m_Initial))
      {
        arraychoices.Add(it.second);
      }

      m_Window = new wxCheckListBox(parent,
        m_Data.window().id(), 
        m_Data.window().pos(), 
        m_Data.window().size(), 
        arraychoices, 
        m_Data.window().style());
      }
      break;

    case CHECKLISTBOX_BOOL:
      {
      wxArrayString arraychoices;
      const std::set<std::string> & choices(std::any_cast<std::set<std::string>>(m_Initial));
      arraychoices.resize(choices.size()); // required!
      std::copy (choices.begin(), choices.end(), arraychoices.begin());
      m_Window = new wxCheckListBox(parent,
        m_Data.window().id(), 
        m_Data.window().pos(), 
        m_Data.window().size(), 
        arraychoices, 
        m_Data.window().style());
      }
      break;

    case COLOURPICKERWIDGET:
      m_Window = new wxColourPickerWidget(parent, 
        m_Data.window().id(), 
        *wxBLACK, 
        m_Data.window().pos(), 
        m_Data.window().size(), 
        m_Data.window().style() == 0 ? wxCLRBTN_DEFAULT_STYLE: m_Data.window().style());
      break;

    case COMBOBOX:
    case COMBOBOX_DIR:
    case COMBOBOX_FILE:
      m_Window = new wxComboBox(parent, 
        m_Data.window().id(), 
        wxEmptyString,
        m_Data.window().pos(), 
        m_Data.window().size(),
        !m_Initial.has_value() ? wxArrayString(): std::any_cast<wxArrayString>(m_Initial),
        m_Data.window().style());
      if (m_Data.validator() != nullptr)
        m_Window->SetValidator(*m_Data.validator());
      break;

    case COMMANDLINKBUTTON:
      m_Window = new wxCommandLinkButton(parent, 
        m_Data.window().id(), 
        before(m_Label, '\t'), 
        after(m_Label, '\t'),
        m_Data.window().pos(), 
        m_Data.window().size(), 
        m_Data.window().style());
      break;

    case DIRPICKERCTRL:
      {
      wxDirPickerCtrl* pc = new wxDirPickerCtrl(parent, 
        m_Data.window().id(), 
        !m_Initial.has_value() ? std::string(): std::any_cast<std::string>(m_Initial),
        wxDirSelectorPromptStr, 
        m_Data.window().pos(), 
        m_Data.window().size(), 
        m_Data.window().style() == 0 ? wxDIRP_DEFAULT_STYLE: m_Data.window().style());

      m_Window = pc;

      if (pc->GetTextCtrl() != nullptr && readonly)
      {
        pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
      }
      }
      break;

    case FILEPICKERCTRL:
      {
#if defined(__WXMSW__) || defined(__OS2__)
      const wxString wc = "*.exe";
#else // Unix/Mac
      const wxString wc(wxFileSelectorDefaultWildcardStr);
#endif

      wxFilePickerCtrl* pc = new wxFilePickerCtrl(parent, 
        m_Data.window().id(), 
        !m_Initial.has_value() ? std::string(): std::any_cast<std::string>(m_Initial),
        wxFileSelectorPromptStr, wc,
        m_Data.window().pos(), 
        m_Data.window().size(),
        m_Data.window().style() == 0 ? wxFLP_DEFAULT_STYLE: m_Data.window().style());

      m_Window = pc;

      if (pc->GetTextCtrl() != nullptr && readonly)
      {
        pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
      }
      }
      break;

    case FONTPICKERCTRL:
      {
      wxFontPickerCtrl* pc = new wxFontPickerCtrl(parent, 
        m_Data.window().id(), 
        wxNullFont,
        m_Data.window().pos(), 
        m_Data.window().size(),
        m_Data.window().style() == 0 ? wxFNTP_FONTDESC_AS_LABEL: m_Data.window().style());

      m_Window = pc;

      if (pc->GetTextCtrl() != nullptr && readonly)
      {
        pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
      }
      }
      break;

    case HYPERLINKCTRL:
#if wxUSE_HYPERLINKCTRL
      m_Window = new wxHyperlinkCtrl(parent, 
        m_Data.window().id(), 
        m_Label,
        std::any_cast<std::string>(m_Initial), 
        m_Data.window().pos(), 
        m_Data.window().size(), 
        m_Data.window().style() == 0 ? wxHL_DEFAULT_STYLE: m_Data.window().style());
#endif      
      break;

    case LISTVIEW:
      {
      listview* lv = new listview(m_ListViewData.
        window(window_data(m_Data.window()).parent(parent)));
      lv->item_from_text(!m_Initial.has_value() ? std::string(): std::any_cast<std::string>(m_Initial));
      m_Window = lv;
      }
      break;

    case NOTEBOOK: 
      bookctrl = new wxNotebook(parent, 
        m_Data.window().id(), 
        m_Data.window().pos(), 
        m_Data.window().size(), 
        m_Data.window().style());
      break;

    case NOTEBOOK_AUI: 
      bookctrl = new wxAuiNotebook(parent,
        m_Data.window().id(), 
        m_Data.window().pos(), 
        m_Data.window().size(), 
        m_Data.window().style() == 0 ? wxAUI_NB_TOP: m_Data.window().style());
      break;

    case NOTEBOOK_CHOICE: 
      bookctrl = new wxChoicebook(parent, 
        m_Data.window().id(), 
        m_Data.window().pos(), 
        m_Data.window().size(), 
        m_Data.window().style());
      break;

    case NOTEBOOK_EX: 
      bookctrl = new notebook(
        m_Data.window(
          window_data().style(wxAUI_NB_TOP).parent(parent)).window()); 
      break;

    case NOTEBOOK_LIST: 
      bookctrl = new wxListbook(parent, 
        m_Data.window().id(), 
        m_Data.window().pos(), 
        m_Data.window().size(), 
        m_Data.window().style());
      break;

    case NOTEBOOK_SIMPLE: 
      bookctrl = new wxSimplebook(parent, 
        m_Data.window().id(), 
        m_Data.window().pos(), 
        m_Data.window().size(), 
        m_Data.window().style());
      break;
          
    case NOTEBOOK_TOOL: 
      bookctrl = new wxToolbook(parent, 
        m_Data.window().id(), 
        m_Data.window().pos(), 
        m_Data.window().size(), 
        m_Data.window().style());

      if (m_ImageList == nullptr)
      {
        log() << "toolbook requires image list";
        return false;
      }
      break;
    
    case NOTEBOOK_TREE: 
      bookctrl = new wxTreebook(parent, 
        m_Data.window().id(), 
        m_Data.window().pos(), 
        m_Data.window().size(), 
        m_Data.window().style());
      break;
    
    case RADIOBOX:
      {
      wxArrayString arraychoices;

      for (const auto& it : std::any_cast<item::choices_t>(m_Initial))
      {
        arraychoices.Add(it.second);
      } 

      m_Window = new wxRadioBox(parent, 
        m_Data.window().id(), 
        m_Label, 
        m_Data.window().pos(), 
        m_Data.window().size(), 
        arraychoices, 
        m_MajorDimension, 
        m_Data.window().style());
      }
      break;

    case SLIDER:
      m_Window = new wxSlider(parent, 
        m_Data.window().id(), 
        std::any_cast<int>(m_Initial),
        std::any_cast<int>(m_Min), 
        std::any_cast<int>(m_Max),
        m_Data.window().pos(), 
        m_Data.window().size(), 
        m_Data.window().style());
      break;

    case SPINCTRL:
      m_Window = new wxSpinCtrl(parent, 
        m_Data.window().id(), 
        wxEmptyString,
        m_Data.window().pos(), 
        m_Data.window().size(),
        wxSP_ARROW_KEYS | (readonly ? wxTE_READONLY: 0),
        std::any_cast<int>(m_Min), 
        std::any_cast<int>(m_Max), 
        !m_Initial.has_value() ? std::any_cast<int>(m_Min): std::any_cast<int>(m_Initial));
      break;

    case SPINCTRLDOUBLE:
      m_Window = new wxSpinCtrlDouble(parent, 
        m_Data.window().id(), 
        wxEmptyString,
        m_Data.window().pos(), 
        m_Data.window().size(),
        wxSP_ARROW_KEYS | (readonly ? wxTE_READONLY: 0),
        std::any_cast<double>(m_Min), 
        std::any_cast<double>(m_Max), 
        !m_Initial.has_value() ? std::any_cast<double>(m_Min): std::any_cast<double>(m_Initial), 
        std::any_cast<double>(m_Inc));
      break;

    case STATICLINE:
      m_Window = new wxStaticLine(parent, 
        m_Data.window().id(),
        m_Data.window().pos(), 
        m_Data.window().size(), 
        m_Data.window().style());
      break;

    case STATICTEXT:
      m_Window = new wxStaticText(parent, 
        m_Data.window().id(), 
        wxEmptyString,
        m_Data.window().pos(),
        m_Data.window().size(),
        m_Data.window().style());
      ((wxStaticText* )m_Window)->SetLabelMarkup(m_Label);
      break;

    case STC:
      m_Window = new stc(std::string(),
        stc_data().
          menu(stc_data::menu_t().set(
            stc_data::MENU_CONTEXT).set(
            stc_data::MENU_OPEN_LINK).set(
            stc_data::MENU_VCS)).
          window(window_data(m_Data.window()).
            parent(parent)));
      
      // Do not use vi mode, as ESC should cancel the dialog,
      // and would not be interpreted by vi.
      ((stc* )m_Window)->get_vi().use(false);

      if (m_Initial.has_value())
      {
        ((stc* )m_Window)->get_lexer().set(std::any_cast<std::string>(m_Initial));
      }
      break;

    case TEXTCTRL:
      m_Window = new wxTextCtrl(parent, 
        m_Data.window().id(), 
        !m_Initial.has_value() ? std::string(): std::any_cast<std::string>(m_Initial),
        m_Data.window().pos(),
        m_Data.window().size(),
        m_Data.window().style() | (readonly ? wxTE_READONLY: 0));
      if (m_Data.validator() != nullptr)
        m_Window->SetValidator(*m_Data.validator());
      break;

    case TEXTCTRL_FLOAT:
      m_Window = new wxTextCtrl(parent, 
        m_Data.window().id(), 
        !m_Initial.has_value() ? std::string(): std::any_cast<std::string>(m_Initial),
        m_Data.window().pos(), 
        m_Data.window().size(), 
        m_Data.window().style() | (readonly ? wxTE_READONLY: 0) | wxTE_RIGHT);
      if (m_Data.validator() == nullptr)
#ifdef __WXMSW__
        // TODO: using wxFloatingPointValidator and wxIntegerValidator
        // gives compile error in MSW.
        ;
#else
        m_Window->SetValidator(wxFloatingPointValidator<double>());
#endif
      else 
        m_Window->SetValidator(*m_Data.validator());
      break;
      
    case TEXTCTRL_INT:
      m_Window = new wxTextCtrl(parent, 
        m_Data.window().id(), 
        !m_Initial.has_value() ? std::string(): std::any_cast<std::string>(m_Initial),
        m_Data.window().pos(), 
        m_Data.window().size(), 
        m_Data.window().style() | (readonly ? wxTE_READONLY: 0) | wxTE_RIGHT);
      if (m_Data.validator() == nullptr)
#ifdef __WXMSW__
        ;
#else
        m_Window->SetValidator(wxIntegerValidator<int>());
#endif
      else 
        m_Window->SetValidator(*m_Data.validator());
      break;

    case TOGGLEBUTTON:
      m_Window = new wxToggleButton(parent, 
        m_Data.window().id(), 
        m_Label,
        m_Data.window().pos(), 
        m_Data.window().size(), 
        m_Data.window().style());
      break;

    case USER:
      if (m_user_window_create_t != nullptr)
      {
        assert(m_Window != nullptr);
        (m_user_window_create_t)(m_Window, parent, readonly);
      }
      break;
  
    default: assert(0);
  }

  if (bookctrl != nullptr)
  {
    m_Window = bookctrl;

    if (m_ImageList != nullptr)
    {
      bookctrl->SetImageList(m_ImageList);
    }
  }
  
  if (m_Type != EMPTY && m_Type != SPACER)
  {
    assert(m_Window != nullptr);
  }

  return true;
}

const std::any wex::item::get_value() const
{
  if (m_Window == nullptr) return std::any();
  
  std::any any;

  try
  {  
    switch (m_Type)
    {
      case CHECKBOX: any = ((wxCheckBox* )m_Window)->GetValue(); break;
      case COLOURPICKERWIDGET: any = ((wxColourPickerWidget* )m_Window)->GetColour(); break;
      case COMBOBOX: 
      case COMBOBOX_DIR: 
      case COMBOBOX_FILE: any = to_container<wxArrayString>((wxComboBox*)m_Window).get(); break;
      case DIRPICKERCTRL: any = ((wxDirPickerCtrl* )m_Window)->GetPath().ToStdString(); break;
      case FILEPICKERCTRL: any = ((wxFilePickerCtrl* )m_Window)->GetPath().ToStdString(); break;
      case FONTPICKERCTRL: any = ((wxFontPickerCtrl* )m_Window)->GetSelectedFont(); break;
      case HYPERLINKCTRL: any = ((wxHyperlinkCtrl* )m_Window)->GetURL().ToStdString(); break;
      case LISTVIEW: any = ((wex::listview* )m_Window)->item_to_text(-1); break;
      case SLIDER: any = ((wxSlider* )m_Window)->GetValue(); break;
      case SPINCTRL: any = ((wxSpinCtrl* )m_Window)->GetValue(); break;
      case SPINCTRLDOUBLE: any = ((wxSpinCtrlDouble* )m_Window)->GetValue(); break;
      case STC: any = ((wxStyledTextCtrl* )m_Window)->GetValue().ToStdString(); break;
      case TEXTCTRL: any = ((wxTextCtrl* )m_Window)->GetValue().ToStdString(); break;
      case TEXTCTRL_FLOAT: any = std::stod(((wxTextCtrl* )m_Window)->GetValue().ToStdString()); break;
      case TEXTCTRL_INT: any = std::stol(((wxTextCtrl* )m_Window)->GetValue().ToStdString()); break;
      case TOGGLEBUTTON: any = ((wxToggleButton* )m_Window)->GetValue(); break;
      
      case CHECKLISTBOX_BIT: {
        auto* clb = (wxCheckListBox*)window();
        long value = 0;
        int item = 0;

        for (const auto& b : std::any_cast<item::choices_t>(m_Initial))
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
    if (!CreateWindow(parent, readonly))
    {
      return nullptr;
    }
    
    wxFlexGridSizer* return_sizer;
    
    switch (m_Type)
    {
      case COMBOBOX_DIR: 
      case COMBOBOX_FILE: 
        return_sizer = AddBrowseButton(sizer);
        break;

      case EMPTY: return fgz;
      case SPACER: sizer->AddSpacer(m_Data.window().style()); return fgz;
      
      default: 
        if (m_Type >= NOTEBOOK && m_Type <= NOTEBOOK_TREE)
        {
          if (!m_Initial.has_value())
          {
            log() << "illegal notebook";
            return nullptr;
          }
          
          wxBookCtrlBase* bookctrl = (wxBookCtrlBase*)m_Window;
          bookctrl->SetName("book-" + m_Label);
          
          return_sizer = Add(sizer, fgz);
          
          // Add all pages and recursive layout the subitems.
          for (auto& page : std::any_cast<items_notebook_t>(m_Initial))
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
          return_sizer = Add(sizer, fgz);
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

  ss << "LABEL: " << m_Label << " " << "TYPE: " << m_Type << " "
     << Log("VALUE: ", get_value()).str()
     << Log("INITIAL: ", m_Initial).str()
     << Log("MIN: ", m_Min).str()
     << Log("MAX: ", m_Max).str()
     << Log("INC: ", m_Inc).str();

  return ss;
}

std::stringstream wex::item::Log(const std::string& name, const std::any& any) const
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

  return s;
}

void wex::item::set_dialog(item_template_dialog<item>* dlg)
{
  m_Dialog = dlg;
}

bool wex::item::set_value(const std::any& value) const
{
  if (m_Window == nullptr || !value.has_value())
  {
    return false;
  }

  try
  {
    switch (m_Type)
    {
      case CHECKBOX: ((wxCheckBox* )m_Window)->SetValue(std::any_cast<bool>(value)); break;
      case COLOURPICKERWIDGET: ((wxColourPickerWidget* )m_Window)->SetColour(std::any_cast<wxColour>(value)); break;
      case COMBOBOX: combobox_as((wxComboBox* )m_Window, std::any_cast<wxArrayString>(value)); break;
      case DIRPICKERCTRL: ((wxDirPickerCtrl* )m_Window)->SetPath(std::any_cast<std::string>(value)); break;
      case FILEPICKERCTRL: ((wxFilePickerCtrl* )m_Window)->SetPath(std::any_cast<std::string>(value)); break;
      case FONTPICKERCTRL: ((wxFontPickerCtrl* )m_Window)->SetSelectedFont(std::any_cast<wxFont>(value)); break;
      case SLIDER: ((wxSlider* )m_Window)->SetValue(std::any_cast<int>(value)); break;
      case SPINCTRL: ((wxSpinCtrl* )m_Window)->SetValue(std::any_cast<int>(value)); break;
      case SPINCTRLDOUBLE: ((wxSpinCtrlDouble* )m_Window)->SetValue(std::any_cast<double>(value)); break;
      case STC: ((wxStyledTextCtrl* )m_Window)->SetValue(std::any_cast<std::string>(value)); break;
      case TEXTCTRL: ((wxTextCtrl* )m_Window)->SetValue(std::any_cast<std::string>(value)); break;
      case TEXTCTRL_FLOAT: ((wxTextCtrl* )m_Window)->SetValue(wxString::Format("%lf", std::any_cast<double>(value))); break;
      case TEXTCTRL_INT: ((wxTextCtrl* )m_Window)->SetValue(wxString::Format("%ld", std::any_cast<long>(value))); break;
      case TOGGLEBUTTON: ((wxToggleButton* )m_Window)->SetValue(std::any_cast<bool>(value)); break;

      case CHECKLISTBOX_BIT:
        {
        auto* clb = (wxCheckListBox*)m_Window;
        int item = 0;

        for (const auto& b : std::any_cast<item::choices_t>(m_Initial))
        {
          clb->Check(item, (std::any_cast<long>(value) & b.first) > 0);
          item++;
        }
        }
        break;
      
      case LISTVIEW:
        {
        auto* win = (listview*)window();
        win->clear();
        win->item_from_text(std::any_cast<std::string>(value));
        }
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
