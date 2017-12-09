////////////////////////////////////////////////////////////////////////////////
// Name:      item.cpp
// Purpose:   Implementation of wxExItem class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <sstream>
#include <easylogging++.h>

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

#include <wx/extension/item.h>
#include <wx/extension/itemtpldlg.h>
#include <wx/extension/listview.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/notebook.h>
#include <wx/extension/stc.h>
#include <wx/extension/tocontainer.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

bool wxExItem::m_UseConfig = true;

wxExItem::wxExItem(wxExItemType type, 
  const wxString& label, const std::any& value, wxExLabelType label_type,
  int major_dimension, const std::any& min, const std::any& max, const std::any& inc,
  wxWindow* window, 
  UserWindowCreate create, UserWindowToConfig config,
  wxImageList* imageList)
  : m_Type(type)
  , m_LabelType(label_type)
  , m_Label(label)
  , m_Initial(value)
  , m_Min(min)
  , m_Max(max)
  , m_Inc(inc)
  , m_MajorDimension(major_dimension)
  , m_UserWindowCreate(create)
  , m_Window(window)
  , m_SizerFlags(wxSizerFlags().Border().Left())
  , m_UserWindowToConfig(config)
  , m_ImageList(imageList)
{
  switch (m_Type)
  {
    case ITEM_CHECKLISTBOX_BIT:
    case ITEM_CHECKLISTBOX_BOOL:
    case ITEM_LISTVIEW:
    case ITEM_NOTEBOOK: 
    case ITEM_NOTEBOOK_AUI: 
    case ITEM_NOTEBOOK_EX: 
    case ITEM_NOTEBOOK_LIST: 
    case ITEM_NOTEBOOK_SIMPLE: 
    case ITEM_NOTEBOOK_TOOL: 
    case ITEM_NOTEBOOK_TREE: 
    case ITEM_RADIOBOX:
    case ITEM_STC:
      m_IsRowGrowable = true;
      m_SizerFlags.Expand();
      break;

    case ITEM_STATICTEXT:
    case ITEM_TEXTCTRL:
    case ITEM_TEXTCTRL_FLOAT:
    case ITEM_TEXTCTRL_INT:
      m_IsRowGrowable = (m_Data.Window().Style() & wxTE_MULTILINE) > 0;
      m_SizerFlags.Expand();
      break;
  
    case ITEM_CHECKBOX:
    case ITEM_COMBOBOX:
    case ITEM_COMBOBOX_DIR:
    case ITEM_COMBOBOX_FILE:
    case ITEM_DIRPICKERCTRL:
    case ITEM_FILEPICKERCTRL:
    case ITEM_SPACER:
    case ITEM_STATICLINE:
    case ITEM_USER:
      m_SizerFlags.Expand();
      break;
    
    default: ; // prevent warning
  }
}

wxFlexGridSizer* wxExItem::Add(wxSizer* sizer, wxFlexGridSizer* current) const
{
  wxASSERT(m_Window != nullptr);

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
  
  if (m_IsRowGrowable && current->GetEffectiveRowsCount() >= 1)
  {
    current->AddGrowableRow(current->GetEffectiveRowsCount() - 1);
  }
  
  return current;
}
      
wxFlexGridSizer* wxExItem::AddBrowseButton(wxSizer* sizer)
{
  wxASSERT(m_Window != nullptr);

  wxFlexGridSizer* fgz = new wxFlexGridSizer(3);
  fgz->AddGrowableCol(1);

  AddStaticText(fgz);

  fgz->Add(m_Window, m_SizerFlags);

  // Tried to use a wxDirPickerCtrl here, is nice,
  // but does not use a combobox for keeping old values, so this is better.
  // And the text box that is used is not resizable as well.
  // In wxExItemTemplateDialog the button click is bound to browse dialog.
  wxButton* browse = new wxButton(
    m_Window->GetParent(),
    m_Window->GetId(),
    _(wxDirPickerWidgetLabel));

  fgz->Add(browse, wxSizerFlags().Center().Border());

  sizer->Add(fgz, wxSizerFlags().Left().Expand()); // no border
  
  return fgz;
}

void wxExItem::AddItems(
  std::pair<wxString, std::vector<wxExItem>> & page, bool readonly)
{
  wxFlexGridSizer* previous_item_sizer = nullptr;
  int previous_type = -1;
  m_Page = page.first;
  int use_cols = 1;
  if (m_MajorDimension != -1) use_cols = m_MajorDimension;
  const size_t col = m_Page.find(":");
  if (col != std::string::npos)
  {
    use_cols = std::stoi(m_Page.substr(col + 1).ToStdString());
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
      LOG(ERROR) << "more pages than images";
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
    // otherwise use no sizer (Layout will create a new one).
    wxFlexGridSizer* current_item_sizer = (
      item.GetType() == previous_type ? previous_item_sizer: nullptr);
    
    item.SetDialog(m_Dialog);
    item.SetImageList(m_ImageList);

    previous_item_sizer = item.Layout(
      bookctrl->GetCurrentPage(),
      booksizer,
      readonly,
      current_item_sizer);
    
    previous_type = item.GetType();
    
    if (m_Dialog != nullptr)
    {
      m_Dialog->Add(item);
      m_Dialog->BindButton(item);
    }

    if (booksizer->GetEffectiveRowsCount() >= 1 &&
       !booksizer->IsRowGrowable(booksizer->GetEffectiveRowsCount() - 1) &&
        item.IsRowGrowable())
    {
      booksizer->AddGrowableRow(booksizer->GetEffectiveRowsCount() - 1);
    }
  }
}
        
wxFlexGridSizer* wxExItem::AddStaticText(wxSizer* sizer) const
{
  wxASSERT(!m_Label.empty());
  wxASSERT(m_Window != nullptr);

  sizer->Add(
    new wxStaticText(m_Window->GetParent(), 
      wxID_ANY, 
      m_Label + ":"), 
      wxSizerFlags().Right().Border().Align(wxALIGN_LEFT));
  
  return nullptr;
}

bool wxExItem::CreateWindow(wxWindow* parent, bool readonly)
{
  if (m_Type != ITEM_USER && m_Window != nullptr)
  {
    return false;
  }
  
  wxBookCtrlBase* bookctrl = nullptr;

  switch (m_Type)
  {
    case ITEM_EMPTY:
    case ITEM_SPACER:
      break;
      
    case ITEM_BUTTON:
      // Using a label is necessary for wxGTK.
      m_Window = new wxButton(parent, 
        m_Data.Window().Id(), 
        "default", 
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        m_Data.Window().Style());
      ((wxButton *)m_Window)->SetLabelMarkup(m_Label);
      break;

    case ITEM_CHECKBOX:
      m_Window = new wxCheckBox(parent, 
        m_Data.Window().Id(), 
        m_Label, 
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        m_Data.Window().Style());
      break;

    case ITEM_CHECKLISTBOX_BIT:
      {
      wxArrayString arraychoices;

      for (const auto& it : std::any_cast<wxExItem::Choices>(m_Initial))
      {
        arraychoices.Add(it.second);
      }

      m_Window = new wxCheckListBox(parent,
        m_Data.Window().Id(), 
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        arraychoices, 
        m_Data.Window().Style());
      }
      break;

    case ITEM_CHECKLISTBOX_BOOL:
      {
      wxArrayString arraychoices;
      const std::set<wxString> & choices(std::any_cast<std::set<wxString>>(m_Initial));
      arraychoices.resize(choices.size()); // required!
      copy (choices.begin(), choices.end(), arraychoices.begin());
      m_Window = new wxCheckListBox(parent,
        m_Data.Window().Id(), 
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        arraychoices, 
        m_Data.Window().Style());
      }
      break;

    case ITEM_COLOURPICKERWIDGET:
      m_Window = new wxColourPickerWidget(parent, 
        m_Data.Window().Id(), 
        *wxBLACK, 
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        m_Data.Window().Style() == 0 ? wxCLRBTN_DEFAULT_STYLE: m_Data.Window().Style());
      break;

    case ITEM_COMBOBOX:
    case ITEM_COMBOBOX_DIR:
    case ITEM_COMBOBOX_FILE:
      m_Window = new wxComboBox(parent, 
        m_Data.Window().Id(), 
        wxEmptyString,
        m_Data.Window().Pos(), 
        m_Data.Window().Size(),
        !m_Initial.has_value() ? wxArrayString(): std::any_cast<wxArrayString>(m_Initial),
        m_Data.Window().Style());
      if (m_Data.Validator() != nullptr)
        m_Window->SetValidator(*m_Data.Validator());
      break;

    case ITEM_COMMANDLINKBUTTON:
      m_Window = new wxCommandLinkButton(parent, 
        m_Data.Window().Id(), 
        m_Label.BeforeFirst('\t'), 
        m_Label.AfterFirst('\t'),
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        m_Data.Window().Style());
      break;

    case ITEM_DIRPICKERCTRL:
      {
      wxDirPickerCtrl* pc = new wxDirPickerCtrl(parent, 
        m_Data.Window().Id(), 
        !m_Initial.has_value() ? wxString(): std::any_cast<wxString>(m_Initial),
        wxDirSelectorPromptStr, 
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        m_Data.Window().Style() == 0 ? wxDIRP_DEFAULT_STYLE: m_Data.Window().Style());

      m_Window = pc;

      if (pc->GetTextCtrl() != nullptr && readonly)
      {
        pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
      }
      }
      break;

    case ITEM_FILEPICKERCTRL:
      {
#if defined(__WXMSW__) || defined(__OS2__)
      const wxString wc = "*.exe";
#else // Unix/Mac
      const wxString wc(wxFileSelectorDefaultWildcardStr);
#endif

      wxFilePickerCtrl* pc = new wxFilePickerCtrl(parent, 
        m_Data.Window().Id(), 
        !m_Initial.has_value() ? wxString(): std::any_cast<wxString>(m_Initial),
        wxFileSelectorPromptStr, wc,
        m_Data.Window().Pos(), 
        m_Data.Window().Size(),
        m_Data.Window().Style() == 0 ? wxFLP_DEFAULT_STYLE: m_Data.Window().Style());

      m_Window = pc;

      if (pc->GetTextCtrl() != nullptr && readonly)
      {
        pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
      }
      }
      break;

    case ITEM_FONTPICKERCTRL:
      {
      wxFontPickerCtrl* pc = new wxFontPickerCtrl(parent, 
        m_Data.Window().Id(), 
        wxNullFont,
        m_Data.Window().Pos(), 
        m_Data.Window().Size(),
        m_Data.Window().Style() == 0 ? wxFNTP_FONTDESC_AS_LABEL: m_Data.Window().Style());

      m_Window = pc;

      if (pc->GetTextCtrl() != nullptr && readonly)
      {
        pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
      }
      }
      break;

    case ITEM_HYPERLINKCTRL:
#if wxUSE_HYPERLINKCTRL
      m_Window = new wxHyperlinkCtrl(parent, 
        m_Data.Window().Id(), 
        m_Label,
        std::any_cast<wxString>(m_Initial), 
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        m_Data.Window().Style() == 0 ? wxHL_DEFAULT_STYLE: m_Data.Window().Style());
#endif      
      break;

    case ITEM_LISTVIEW:
      {
      wxExListView* lv = new wxExListView(m_ListViewData.
        Window(wxExWindowData(m_Data.Window()).Parent(parent)));
      lv->ItemFromText(!m_Initial.has_value() ? std::string(): std::any_cast<std::string>(m_Initial));
      m_Window = lv;
      }
      break;

    case ITEM_NOTEBOOK: 
      bookctrl = new wxNotebook(parent, 
        m_Data.Window().Id(), 
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        m_Data.Window().Style());
      break;

    case ITEM_NOTEBOOK_AUI: 
      bookctrl = new wxAuiNotebook(parent,
        m_Data.Window().Id(), 
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        m_Data.Window().Style() == 0 ? wxAUI_NB_TOP: m_Data.Window().Style());
      break;

    case ITEM_NOTEBOOK_CHOICE: 
      bookctrl = new wxChoicebook(parent, 
        m_Data.Window().Id(), 
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        m_Data.Window().Style());
      break;

    case ITEM_NOTEBOOK_EX: 
      bookctrl = new wxExNotebook(
        m_Data.Window(
          wxExWindowData().Style(wxAUI_NB_TOP).Parent(parent)).Window()); 
      break;

    case ITEM_NOTEBOOK_LIST: 
      bookctrl = new wxListbook(parent, 
        m_Data.Window().Id(), 
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        m_Data.Window().Style());
      break;

    case ITEM_NOTEBOOK_SIMPLE: 
      bookctrl = new wxSimplebook(parent, 
        m_Data.Window().Id(), 
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        m_Data.Window().Style());
      break;
          
    case ITEM_NOTEBOOK_TOOL: 
      bookctrl = new wxToolbook(parent, 
        m_Data.Window().Id(), 
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        m_Data.Window().Style());

      if (m_ImageList == nullptr)
      {
        LOG(ERROR) << "toolbook requires image list";
        return false;
      }
      break;
    
    case ITEM_NOTEBOOK_TREE: 
      bookctrl = new wxTreebook(parent, 
        m_Data.Window().Id(), 
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        m_Data.Window().Style());
      break;
    
    case ITEM_RADIOBOX:
      {
      wxArrayString arraychoices;

      for (const auto& it : std::any_cast<wxExItem::Choices>(m_Initial))
      {
        arraychoices.Add(it.second);
      } 

      m_Window = new wxRadioBox(parent, 
        m_Data.Window().Id(), 
        m_Label, 
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        arraychoices, 
        m_MajorDimension, 
        m_Data.Window().Style());
      }
      break;

    case ITEM_SLIDER:
      m_Window = new wxSlider(parent, 
        m_Data.Window().Id(), 
        std::any_cast<int>(m_Initial),
        std::any_cast<int>(m_Min), 
        std::any_cast<int>(m_Max),
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        m_Data.Window().Style());
      break;

    case ITEM_SPINCTRL:
      m_Window = new wxSpinCtrl(parent, 
        m_Data.Window().Id(), 
        wxEmptyString,
        m_Data.Window().Pos(), 
        m_Data.Window().Size(),
        wxSP_ARROW_KEYS | (readonly ? wxTE_READONLY: 0),
        std::any_cast<int>(m_Min), 
        std::any_cast<int>(m_Max), 
        !m_Initial.has_value() ? std::any_cast<int>(m_Min): std::any_cast<int>(m_Initial));
      break;

    case ITEM_SPINCTRLDOUBLE:
      m_Window = new wxSpinCtrlDouble(parent, 
        m_Data.Window().Id(), 
        wxEmptyString,
        m_Data.Window().Pos(), 
        m_Data.Window().Size(),
        wxSP_ARROW_KEYS | (readonly ? wxTE_READONLY: 0),
        std::any_cast<double>(m_Min), 
        std::any_cast<double>(m_Max), 
        !m_Initial.has_value() ? std::any_cast<double>(m_Min): std::any_cast<double>(m_Initial), 
        std::any_cast<double>(m_Inc));
      break;

    case ITEM_STATICLINE:
      m_Window = new wxStaticLine(parent, 
        m_Data.Window().Id(),
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        m_Data.Window().Style());
      break;

    case ITEM_STATICTEXT:
      m_Window = new wxStaticText(parent, 
        m_Data.Window().Id(), 
        wxEmptyString,
        m_Data.Window().Pos(),
        m_Data.Window().Size(),
        m_Data.Window().Style());
      ((wxStaticText* )m_Window)->SetLabelMarkup(m_Label);
      break;

    case ITEM_STC:
      m_Window = new wxExSTC(std::string(),
        wxExSTCData().
          Menu(static_cast<wxExSTCMenuFlags>(STC_MENU_CONTEXT | STC_MENU_OPEN_LINK | STC_MENU_VCS)).
          Window(wxExWindowData(m_Data.Window()).
            Parent(parent)));
      
      // Do not use vi mode, as ESC should cancel the dialog,
      // and would not be interpreted by vi.
      ((wxExSTC* )m_Window)->GetVi().Use(false);

      if (m_Initial.has_value())
      {
        ((wxExSTC* )m_Window)->GetLexer().Set(std::any_cast<wxString>(m_Initial).ToStdString());
      }
      break;

    case ITEM_TEXTCTRL:
      m_Window = new wxTextCtrl(parent, 
        m_Data.Window().Id(), 
        !m_Initial.has_value() ? wxString(): std::any_cast<wxString>(m_Initial),
        m_Data.Window().Pos(),
        m_Data.Window().Size(),
        m_Data.Window().Style() | (readonly ? wxTE_READONLY: 0));
      if (m_Data.Validator() != nullptr)
        m_Window->SetValidator(*m_Data.Validator());
      break;

    case ITEM_TEXTCTRL_FLOAT:
      m_Window = new wxTextCtrl(parent, 
        m_Data.Window().Id(), 
        !m_Initial.has_value() ? wxString(): std::any_cast<wxString>(m_Initial),
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        m_Data.Window().Style() | (readonly ? wxTE_READONLY: 0) | wxTE_RIGHT);
      if (m_Data.Validator() == nullptr)
#ifdef __WXMSW__
        // TODO: using wxFloatingPointValidator and wxIntegerValidator
        // gives compile error in MSW.
        ;
#else
        m_Window->SetValidator(wxFloatingPointValidator<double>());
#endif
      else 
        m_Window->SetValidator(*m_Data.Validator());
      break;
      
    case ITEM_TEXTCTRL_INT:
      m_Window = new wxTextCtrl(parent, 
        m_Data.Window().Id(), 
        !m_Initial.has_value() ? wxString(): std::any_cast<wxString>(m_Initial),
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        m_Data.Window().Style() | (readonly ? wxTE_READONLY: 0) | wxTE_RIGHT);
      if (m_Data.Validator() == nullptr)
#ifdef __WXMSW__
        ;
#else
        m_Window->SetValidator(wxIntegerValidator<int>());
#endif
      else 
        m_Window->SetValidator(*m_Data.Validator());
      break;

    case ITEM_TOGGLEBUTTON:
      m_Window = new wxToggleButton(parent, 
        m_Data.Window().Id(), 
        m_Label,
        m_Data.Window().Pos(), 
        m_Data.Window().Size(), 
        m_Data.Window().Style());
      break;

    case ITEM_USER:
      if (m_UserWindowCreate != nullptr)
      {
        wxASSERT(m_Window != nullptr);
        (m_UserWindowCreate)(m_Window, parent, readonly);
      }
      break;
  
    default: wxFAIL;
  }

  if (bookctrl != nullptr)
  {
    m_Window = bookctrl;

    if (m_ImageList != nullptr)
    {
      bookctrl->SetImageList(m_ImageList);
    }
  }
  
  if (m_Type != ITEM_EMPTY && m_Type != ITEM_SPACER)
  {
    wxASSERT(m_Window != nullptr);
  }

  return true;
}

const std::any wxExItem::GetValue() const
{
  if (m_Window == nullptr) return std::any();
  
  std::any any;
  
  switch (m_Type)
  {
    case ITEM_CHECKBOX: any = ((wxCheckBox* )m_Window)->GetValue(); break;
    case ITEM_COLOURPICKERWIDGET: any = ((wxColourPickerWidget* )m_Window)->GetColour(); break;
    case ITEM_COMBOBOX: 
    case ITEM_COMBOBOX_DIR: 
    case ITEM_COMBOBOX_FILE: any = wxExToContainer<wxArrayString>((wxComboBox*)m_Window).Get(); break;
    case ITEM_DIRPICKERCTRL: any = ((wxDirPickerCtrl* )m_Window)->GetPath(); break;
    case ITEM_FILEPICKERCTRL: any = ((wxFilePickerCtrl* )m_Window)->GetPath(); break;
    case ITEM_FONTPICKERCTRL: any = ((wxFontPickerCtrl* )m_Window)->GetSelectedFont(); break;
    case ITEM_HYPERLINKCTRL: any = ((wxHyperlinkCtrl* )m_Window)->GetURL(); break;
    case ITEM_LISTVIEW: any = ((wxExListView* )m_Window)->ItemToText(-1); break;
    case ITEM_SLIDER: any = ((wxSlider* )m_Window)->GetValue(); break;
    case ITEM_SPINCTRL: any = ((wxSpinCtrl* )m_Window)->GetValue(); break;
    case ITEM_SPINCTRLDOUBLE: any = ((wxSpinCtrlDouble* )m_Window)->GetValue(); break;
    case ITEM_STC: any = ((wxStyledTextCtrl* )m_Window)->GetValue(); break;
    case ITEM_TEXTCTRL: any = ((wxTextCtrl* )m_Window)->GetValue(); break;
    case ITEM_TEXTCTRL_FLOAT: any = std::stod(((wxTextCtrl* )m_Window)->GetValue().ToStdString()); break;
    case ITEM_TEXTCTRL_INT: any = std::stol(((wxTextCtrl* )m_Window)->GetValue().ToStdString()); break;
    case ITEM_TOGGLEBUTTON: any = ((wxToggleButton* )m_Window)->GetValue(); break;
    
    case ITEM_CHECKLISTBOX_BIT: {
      wxCheckListBox* clb = (wxCheckListBox*)GetWindow();
      long value = 0;
      int item = 0;

      for (const auto& b : std::any_cast<wxExItem::Choices>(m_Initial))
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
  
  return any;
}
  
wxFlexGridSizer* wxExItem::Layout(
  wxWindow* parent, 
  wxSizer* sizer, 
  bool readonly,
  wxFlexGridSizer* fgz)
{
  wxASSERT(sizer != nullptr);

  try
  {  
    if (!CreateWindow(parent, readonly))
    {
      return nullptr;
    }
    
    wxFlexGridSizer* return_sizer;
    
    switch (m_Type)
    {
      case ITEM_COMBOBOX_DIR: 
      case ITEM_COMBOBOX_FILE: 
        return_sizer = AddBrowseButton(sizer); break;
      case ITEM_EMPTY: return fgz;
      case ITEM_SPACER: sizer->AddSpacer(m_Data.Window().Style()); return fgz;
      
      default: 
        if (m_Type >= ITEM_NOTEBOOK && m_Type <= ITEM_NOTEBOOK_TREE)
        {
          if (!m_Initial.has_value())
          {
            LOG(ERROR) << "illegal notebook";
            return nullptr;
          }
          
          wxBookCtrlBase* bookctrl = (wxBookCtrlBase*)m_Window;
          bookctrl->SetName("book-" + m_Label);
          
          return_sizer = Add(sizer, fgz);
          
          // Add all pages and recursive layout the subitems.
          for (auto& page : std::any_cast<ItemsNotebook>(m_Initial))
          {
            AddItems(page, readonly);
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
    
    ToConfig(false);

    return return_sizer;
  }
  catch (std::bad_cast& e)
  {
    LOG(ERROR) << "item: " << e.what() << Log().str();
  }
  
  return nullptr;
}

std::stringstream wxExItem::Log() const
{
  std::stringstream ss;

  ss << "LABEL: " << m_Label << " " << "TYPE: " << m_Type << " "
     << Log("VALUE: ", GetValue()).str()
     << Log("INITIAL: ", m_Initial).str()
     << Log("MIN: ", m_Min).str()
     << Log("MAX: ", m_Max).str()
     << Log("INC: ", m_Inc).str();

  return ss;
}

std::stringstream wxExItem::Log(const std::string& name, const std::any& any) const
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
      else if (any.type() == typeid(wxString)) 
      {
        s << std::any_cast<wxString>(any);
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
      s << "<bad cast: " << e.what() << ">";
    }
  }
  else
  {
    s << "<no value>";
  }

  s << "} ";

  return s;
}

void wxExItem::SetDialog(wxExItemTemplateDialog<wxExItem>* dlg)
{
  m_Dialog = dlg;
}

bool wxExItem::SetValue(const std::any& value) const
{
  if (m_Window == nullptr || !value.has_value())
  {
    return false;
  }

  switch (m_Type)
  {
    case ITEM_CHECKBOX: ((wxCheckBox* )m_Window)->SetValue(std::any_cast<bool>(value)); break;
    case ITEM_COLOURPICKERWIDGET: ((wxColourPickerWidget* )m_Window)->SetColour(std::any_cast<wxColour>(value)); break;
    case ITEM_COMBOBOX: wxExComboBoxAs((wxComboBox* )m_Window, std::any_cast<wxArrayString>(value)); break;
    case ITEM_DIRPICKERCTRL: ((wxDirPickerCtrl* )m_Window)->SetPath(std::any_cast<wxString>(value)); break;
    case ITEM_FILEPICKERCTRL: ((wxFilePickerCtrl* )m_Window)->SetPath(std::any_cast<wxString>(value)); break;
    case ITEM_FONTPICKERCTRL: ((wxFontPickerCtrl* )m_Window)->SetSelectedFont(std::any_cast<wxFont>(value)); break;
    case ITEM_SLIDER: ((wxSlider* )m_Window)->SetValue(std::any_cast<int>(value)); break;
    case ITEM_SPINCTRL: ((wxSpinCtrl* )m_Window)->SetValue(std::any_cast<int>(value)); break;
    case ITEM_SPINCTRLDOUBLE: ((wxSpinCtrlDouble* )m_Window)->SetValue(std::any_cast<double>(value)); break;
    case ITEM_STC: ((wxStyledTextCtrl* )m_Window)->SetValue(std::any_cast<wxString>(value)); break;
    case ITEM_TEXTCTRL: ((wxTextCtrl* )m_Window)->SetValue(std::any_cast<wxString>(value)); break;
    case ITEM_TEXTCTRL_FLOAT: ((wxTextCtrl* )m_Window)->SetValue(wxString::Format("%lf", std::any_cast<double>(value))); break;
    case ITEM_TEXTCTRL_INT: ((wxTextCtrl* )m_Window)->SetValue(wxString::Format("%ld", std::any_cast<long>(value))); break;
    case ITEM_TOGGLEBUTTON: ((wxToggleButton* )m_Window)->SetValue(std::any_cast<bool>(value)); break;

    case ITEM_CHECKLISTBOX_BIT:
      {
      wxCheckListBox* clb = (wxCheckListBox*)m_Window;
      int item = 0;

      for (const auto& b : std::any_cast<wxExItem::Choices>(m_Initial))
      {
        clb->Check(item, (std::any_cast<long>(value) & b.first) > 0);
        item++;
      }
      }
      break;
    
    case ITEM_LISTVIEW:
      {
      wxExListView* win = (wxExListView*)GetWindow();
      win->DeleteAllItems();
      win->ItemFromText(std::any_cast<std::string>(value));
      }
      break;

    default: return false;
  }
  
  return true;
}

#endif // wxUSE_GUI
