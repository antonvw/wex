////////////////////////////////////////////////////////////////////////////////
// Name:      item.cpp
// Purpose:   Implementation of wxExItem class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

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
#include <wx/valnum.h>
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
#include <wx/extension/util.h>

#if wxUSE_GUI

bool wxExItem::m_UseConfig = true;

wxExItem::wxExItem(wxExItemType type, long style,
  const wxString& label, const wxAny& value,
  bool is_required, wxExLabelType label_type,
  int id, int major_dimension,
  const wxAny& min, const wxAny& max, const wxAny& inc,
  wxWindow* window, wxExUserWindowCreate create,
  wxExUserWindowToConfig config,
  wxImageList* imageList)
  : m_Type(type)
  , m_Style(style)
  , m_LabelType(label_type)
  , m_Label(label)
  , m_Initial(value)
  , m_IsRequired(is_required)
  , m_Id(id)
  , m_Min(min)
  , m_Max(max)
  , m_Inc(inc)
  , m_MajorDimension(major_dimension)
  , m_UserWindowCreate(create)
  , m_Validator(nullptr)
  , m_Window(window)
  , m_IsRowGrowable(false)
  , m_Page()
  , m_SizerFlags(wxSizerFlags().Border().Left())
  , m_MaxItems(25)
  , m_UserWindowToConfig(config)
  , m_ImageList(imageList)
  , m_Dialog(nullptr)
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
      m_IsRowGrowable = (m_Style & wxTE_MULTILINE) > 0;
      m_SizerFlags.Expand();
      break;
  
    case ITEM_CHECKBOX:
    case ITEM_COMBOBOX:
    case ITEM_COMBOBOX_DIR:
    case ITEM_DIRPICKERCTRL:
    case ITEM_FILEPICKERCTRL:
    case ITEM_SPACER:
    case ITEM_STATICLINE:
    case ITEM_USER:
      m_SizerFlags.Expand();
      break;
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
      
wxFlexGridSizer* wxExItem::AddBrowseButton(wxSizer* sizer) const
{
  wxASSERT(m_Window != nullptr);

  wxFlexGridSizer* fgz = new wxFlexGridSizer(3);
  fgz->AddGrowableCol(1);

  AddStaticText(fgz);

  fgz->Add(m_Window, m_SizerFlags);

  // Tried to use a wxDirPickerCtrl here, is nice,
  // but does not use a combobox for keeping old values, so this is better.
  // And the text box that is used is not resizable as well.
  fgz->Add(
    new wxButton(
      m_Window->GetParent(),
      m_Window->GetId(),
      _(wxDirPickerWidgetLabel)),
    wxSizerFlags().Center().Border());

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
  if (m_Page.Contains(":"))
  {
    use_cols = atoi(page.first.AfterFirst(':'));
    m_Page = page.first.BeforeFirst(':');
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
      wxLogError("more pages than images");
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
  
  for (auto & item: page.second)
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
  if (m_Window != nullptr)
  {
    return false;
  }
  
  const int width = 200;
  const int width_numeric = 75;
  const int width_numeric_spin = 125;
  wxBookCtrlBase* bookctrl = nullptr;
  
  switch (m_Type)
  {
    case ITEM_EMPTY:
    case ITEM_SPACER:
      break;
      
    case ITEM_BUTTON:
      // Using a label is necessary for wxGTK.
      m_Window = new wxButton(parent, m_Id, "default", 
        wxDefaultPosition, wxDefaultSize, m_Style);
      ((wxButton *)m_Window)->SetLabelMarkup(m_Label);
      break;

    case ITEM_CHECKBOX:
      m_Window = new wxCheckBox(parent, m_Id, m_Label, 
        wxDefaultPosition, wxDefaultSize, m_Style);
      break;

    case ITEM_CHECKLISTBOX_BIT:
      {
      wxArrayString arraychoices;

      for (const auto & it : m_Initial.As<std::map<long, const wxString>>())
      {
        arraychoices.Add(it.second);
      }

      m_Window = new wxCheckListBox(parent,
        m_Id, wxDefaultPosition, wxDefaultSize, arraychoices, m_Style);
      }
      break;

    case ITEM_CHECKLISTBOX_BOOL:
      {
      wxArrayString arraychoices;
      const std::set<wxString> & choices(m_Initial.As<std::set<wxString>>());
      arraychoices.resize(choices.size()); // required!
      copy (choices.begin(), choices.end(), arraychoices.begin());
      m_Window = new wxCheckListBox(parent,
        m_Id, wxDefaultPosition, wxDefaultSize, arraychoices, m_Style);
      }
      break;

    case ITEM_COLOURPICKERWIDGET:
      m_Window = new wxColourPickerWidget(parent, m_Id, 
        *wxBLACK, wxDefaultPosition, wxDefaultSize, wxCLRBTN_DEFAULT_STYLE); // no m_Style
      break;

    case ITEM_COMBOBOX:
    case ITEM_COMBOBOX_DIR:
      m_Window = new wxComboBox(parent, m_Id, wxEmptyString,
        wxDefaultPosition, wxSize(250, wxDefaultCoord),
        m_Initial.IsNull() ? wxArrayString(): m_Initial.As<wxArrayString>(),
        0,
        (m_Validator != nullptr ? *m_Validator: wxDefaultValidator));
      break;

    case ITEM_COMMANDLINKBUTTON:
      m_Window = new wxCommandLinkButton(parent, m_Id, 
        m_Label.BeforeFirst('\t'), m_Label.AfterFirst('\t'),
        wxDefaultPosition, wxDefaultSize, m_Style);
      break;

    case ITEM_DIRPICKERCTRL:
      {
      wxDirPickerCtrl* pc = new wxDirPickerCtrl(parent, m_Id, 
        m_Initial.IsNull() ? wxString(): m_Initial.As<wxString>(),
        wxDirSelectorPromptStr, wxDefaultPosition, wxSize(width, wxDefaultCoord), 
        m_Style == 0 ? wxDIRP_DEFAULT_STYLE: m_Style);

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

      wxFilePickerCtrl* pc = new wxFilePickerCtrl(parent, m_Id, 
        m_Initial.IsNull() ? wxString(): m_Initial.As<wxString>(),
        wxFileSelectorPromptStr, wc,
        wxDefaultPosition, wxSize(width, wxDefaultCoord),
        m_Style == 0 ? wxFLP_DEFAULT_STYLE: m_Style);

      m_Window = pc;

      if (pc->GetTextCtrl() != nullptr && readonly)
      {
        pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
      }
      }
      break;

    case ITEM_TEXTCTRL_FLOAT:
      m_Window = new wxTextCtrl(parent, m_Id, 
        m_Initial.IsNull() ? wxString(): m_Initial.As<wxString>(),
        wxDefaultPosition, wxSize(width_numeric, wxDefaultCoord),
        m_Style | (readonly ? wxTE_READONLY: 0) | wxTE_RIGHT,
        m_Validator == nullptr ? wxFloatingPointValidator<float>(): *m_Validator);
      break;
      
    case ITEM_FONTPICKERCTRL:
      {
      wxFontPickerCtrl* pc = new wxFontPickerCtrl(parent, m_Id, wxNullFont,
        wxDefaultPosition, wxDefaultSize,
        wxFNTP_FONTDESC_AS_LABEL); // no wxFNTP_USEFONT_FOR_LABEL

      m_Window = pc;

      if (pc->GetTextCtrl() != nullptr && readonly)
      {
        pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
      }
      }
      break;

    case ITEM_HYPERLINKCTRL:
#if wxUSE_HYPERLINKCTRL
      m_Window = new wxHyperlinkCtrl(parent, m_Id, m_Label,
        m_Initial.As<wxString>(), wxDefaultPosition, wxSize(width, wxDefaultCoord), wxHL_DEFAULT_STYLE); // no m_Style
#endif      
      break;

    case ITEM_TEXTCTRL_INT:
      m_Window = new wxTextCtrl(parent, m_Id, 
        m_Initial.IsNull() ? wxString(): m_Initial.As<wxString>(),
        wxDefaultPosition, wxSize(width_numeric, wxDefaultCoord),
        m_Style | (readonly ? wxTE_READONLY: 0) | wxTE_RIGHT,
        m_Validator == nullptr ? wxIntegerValidator<int>(): *m_Validator);
      break;

    case ITEM_LISTVIEW:
      {
      wxExListView* lv = new wxExListView(parent, (wxExListView::wxExListType)m_Style, m_Id, nullptr,
        wxDefaultPosition, wxSize(width, 200));
      lv->ItemFromText(m_Initial.IsNull() ? wxString(): m_Initial.As<wxString>());
      m_Window = lv;
      }
      break;

    case ITEM_NOTEBOOK: bookctrl = new wxNotebook(parent, wxID_ANY); break;
    case ITEM_NOTEBOOK_AUI: bookctrl = new wxAuiNotebook(parent,
      wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP); break;
    case ITEM_NOTEBOOK_CHOICE: bookctrl = new wxChoicebook(parent, wxID_ANY); break;
    case ITEM_NOTEBOOK_EX: bookctrl = new wxExNotebook(parent, 
      dynamic_cast<wxExManagedFrame*>(wxTheApp->GetTopWindow()),
      wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_NB_TOP); break;
    case ITEM_NOTEBOOK_LIST: bookctrl = new wxListbook(parent, wxID_ANY); break;
    case ITEM_NOTEBOOK_SIMPLE: bookctrl = new wxSimplebook(parent, wxID_ANY); break;
    case ITEM_NOTEBOOK_TREE: bookctrl = new wxTreebook(parent, wxID_ANY); break;
    
    case ITEM_NOTEBOOK_TOOL: bookctrl = new wxToolbook(parent, wxID_ANY);
      if (m_ImageList == nullptr)
      {
        wxLogError("toolbook requires image list");
        return false;
      }
      break;
    
    case ITEM_RADIOBOX:
      {
      wxArrayString arraychoices;

      for (const auto& it : m_Initial.As<std::map<long, const wxString>>())
      {
        arraychoices.Add(it.second);
      } 

      m_Window = new wxRadioBox(parent, m_Id, m_Label, 
        wxDefaultPosition, wxDefaultSize, arraychoices, m_MajorDimension, m_Style);
      }
      break;

    case ITEM_SLIDER:
      m_Window = new wxSlider(parent, m_Id, m_Initial.As<int>(),
        m_Min.As<int>(), m_Max.As<int>(),
        wxDefaultPosition, wxSize(width_numeric, wxDefaultCoord), m_Style);
      break;

    case ITEM_SPINCTRL:
      m_Window = new wxSpinCtrl(parent, m_Id, wxEmptyString,
        wxDefaultPosition, wxSize(width_numeric_spin, wxDefaultCoord),
        wxSP_ARROW_KEYS | (readonly ? wxTE_READONLY: 0),
        m_Min.As<int>(), m_Max.As<int>(), 
        m_Initial.IsNull() ? m_Min.As<int>(): m_Initial.As<int>());
      break;

    case ITEM_SPINCTRLDOUBLE:
      m_Window = new wxSpinCtrlDouble(parent, m_Id, wxEmptyString,
        wxDefaultPosition, wxSize(width_numeric_spin, wxDefaultCoord),
        wxSP_ARROW_KEYS | (readonly ? wxTE_READONLY: 0),
        m_Min.As<double>(), m_Max.As<double>(), 
        m_Initial.IsNull() ? m_Min.As<double>(): m_Initial.As<double>(), m_Inc.As<double>());
      break;

    case ITEM_STATICLINE:
      m_Window = new wxStaticLine(parent, m_Id,
        wxDefaultPosition, wxDefaultSize, m_Style);
      break;

    case ITEM_STATICTEXT:
      m_Window = new wxStaticText(parent, m_Id, wxEmptyString,
        wxDefaultPosition,
        (m_Style & wxTE_MULTILINE ?
           wxSize(width, 200):
           wxSize(width, wxDefaultCoord)),
        m_Style);
      ((wxStaticText* )m_Window)->SetLabelMarkup(m_Label);
      break;

    case ITEM_STC:
      m_Window = new wxExSTC(parent, wxEmptyString, 0,
        wxEmptyString, wxExSTC::STC_MENU_DEFAULT, std::string(), m_Id);
      
      // Do not use vi mode, as ESC should cancel the dialog,
      // and would not be interpreted by vi.
      ((wxExSTC* )m_Window)->GetVi().Use(false);

      if (!m_Initial.IsNull())
      {
        ((wxExSTC* )m_Window)->GetLexer().Set(m_Initial.As<wxString>());
      }
      break;

    case ITEM_TEXTCTRL:
      m_Window = new wxTextCtrl(parent, m_Id, 
        m_Initial.IsNull() ? wxString(): m_Initial.As<wxString>(),
        wxDefaultPosition,
        (m_Style & wxTE_MULTILINE ?
           wxSize(width, 200):
           wxSize(width, wxDefaultCoord)),
        m_Style | 
          (readonly ? wxTE_READONLY: 0),
        (m_Validator != nullptr ? *m_Validator: wxDefaultValidator));
      break;

    case ITEM_TOGGLEBUTTON:
      m_Window = new wxToggleButton(parent, m_Id, m_Label,
        wxDefaultPosition, wxDefaultSize, m_Style);
      break;

    case ITEM_USER:
      wxASSERT(m_Window != nullptr);
    
      if (m_UserWindowCreate != nullptr)
      {
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

const wxAny wxExItem::GetValue() const
{
  wxAny any;
  
  if (m_Window == nullptr)
  {
    return any;
  }
  
  switch (m_Type)
  {
    case ITEM_CHECKBOX: any = ((wxCheckBox* )m_Window)->GetValue(); break;
    case ITEM_COLOURPICKERWIDGET: any = ((wxColourPickerWidget* )m_Window)->GetColour(); break;
    case ITEM_COMBOBOX: any = wxExComboBoxAs<wxArrayString>((wxComboBox*)m_Window); break;
    case ITEM_DIRPICKERCTRL: any = ((wxDirPickerCtrl* )m_Window)->GetPath(); break;
    case ITEM_FILEPICKERCTRL: any = ((wxFilePickerCtrl* )m_Window)->GetPath(); break;
    case ITEM_TEXTCTRL_FLOAT: any = atof(((wxTextCtrl* )m_Window)->GetValue()); break;
    case ITEM_FONTPICKERCTRL: any = ((wxFontPickerCtrl* )m_Window)->GetSelectedFont(); break;
    case ITEM_TEXTCTRL_INT: any = atoi(((wxTextCtrl* )m_Window)->GetValue()); break;
    case ITEM_LISTVIEW: any = ((wxExListView* )m_Window)->ItemToText(-1); break;
    case ITEM_SLIDER: any = ((wxSlider* )m_Window)->GetValue(); break;
    case ITEM_SPINCTRL: any = ((wxSpinCtrl* )m_Window)->GetValue(); break;
    case ITEM_SPINCTRLDOUBLE: any = ((wxSpinCtrlDouble* )m_Window)->GetValue(); break;
    case ITEM_STC: any = ((wxStyledTextCtrl* )m_Window)->GetValue(); break;
    case ITEM_TEXTCTRL: any = ((wxTextCtrl* )m_Window)->GetValue(); break;
    case ITEM_TOGGLEBUTTON: any = ((wxToggleButton* )m_Window)->GetValue(); break;
    
    case ITEM_CHECKLISTBOX_BIT:
      {
      wxCheckListBox* clb = (wxCheckListBox*)GetWindow();
      long value = 0;
      int item = 0;
      for (const auto& b : m_Initial.As<std::map<long, const wxString>>())
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
  
  if (!CreateWindow(parent, readonly))
  {
    return nullptr;
  }
  
  wxFlexGridSizer* return_sizer;
  
  switch (m_Type)
  {
    case ITEM_COMBOBOX_DIR: return_sizer = AddBrowseButton(sizer); break;
    case ITEM_EMPTY: return fgz;
    case ITEM_SPACER: sizer->AddSpacer(m_Style); return fgz;
    
    default: 
      if (IsNotebook())
      {
        if (m_Initial.IsNull() || !m_Initial.CheckType<ItemsNotebook>())
        {
          wxLogError("Illegal notebook");
          return nullptr;
        }
        
        wxBookCtrlBase* bookctrl = (wxBookCtrlBase*)m_Window;
        bookctrl->SetName("book-" + m_Label);
        
        return_sizer = Add(sizer, fgz);
        
        // Add all pages and recursive layout the subitems.
        for (auto & page : m_Initial.As<ItemsNotebook>())
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
      
        /* TODO: next causes inifinite recursion, use different sizers for each page.
        if (bookctrl->GetCurrentPage() != nullptr)
        {
          bookctrl->GetCurrentPage()->SetSizer(sizer);
        } */      
      }
      else 
      {
        return_sizer = Add(sizer, fgz);
      }
  }
  
  ToConfig(false);
  
  return return_sizer;
}

void wxExItem::SetDialog(wxExItemTemplateDialog<wxExItem>* dlg)
{
  m_Dialog = dlg;
}

bool wxExItem::SetValue(const wxAny& value) const
{
  if (m_Window == nullptr)
  {
    return false;
  }

  switch (m_Type)
  {
    case ITEM_CHECKBOX: ((wxCheckBox* )m_Window)->SetValue(value.As<bool>()); break;
    case ITEM_COLOURPICKERWIDGET: ((wxColourPickerWidget* )m_Window)->SetColour(value.As<wxColour>()); break;
    case ITEM_COMBOBOX: wxExComboBoxAs((wxComboBox* )m_Window, value.As<wxArrayString>()); break;
    case ITEM_DIRPICKERCTRL: ((wxDirPickerCtrl* )m_Window)->SetPath(value.As<wxString>()); break;
    case ITEM_FILEPICKERCTRL: ((wxFilePickerCtrl* )m_Window)->SetPath(value.As<wxString>()); break;
    case ITEM_TEXTCTRL_FLOAT: ((wxTextCtrl* )m_Window)->SetValue(wxString::Format("%lf", value.As<float>())); break;
    case ITEM_FONTPICKERCTRL: ((wxFontPickerCtrl* )m_Window)->SetSelectedFont(value.As<wxFont>()); break;
    case ITEM_TEXTCTRL_INT: ((wxTextCtrl* )m_Window)->SetValue(wxString::Format("%ld", value.As<long>())); break;
    case ITEM_SLIDER: ((wxSlider* )m_Window)->SetValue(value.As<int>()); break;
    case ITEM_SPINCTRL: ((wxSpinCtrl* )m_Window)->SetValue(value.As<int>()); break;
    case ITEM_SPINCTRLDOUBLE: ((wxSpinCtrlDouble* )m_Window)->SetValue(value.As<double>()); break;
    case ITEM_STC: ((wxStyledTextCtrl* )m_Window)->SetValue(value.As<wxString>()); break;
    case ITEM_TEXTCTRL: ((wxTextCtrl* )m_Window)->SetValue(value.As<wxString>()); break;
    case ITEM_TOGGLEBUTTON: ((wxToggleButton* )m_Window)->SetValue(value.As<bool>()); break;

    case ITEM_CHECKLISTBOX_BIT:
      {
      wxCheckListBox* clb = (wxCheckListBox*)m_Window;
      int item = 0;

      for (const auto& b : m_Initial.As<std::map<long, const wxString>>())
      {
        clb->Check(item, (value.As<long>() & b.first) > 0);
        item++;
      }
      }
      break;
    
    case ITEM_LISTVIEW:
      {
      wxExListView* win = (wxExListView*)GetWindow();
      win->DeleteAllItems();
      win->ItemFromText(value.As<wxString>());
      }
      break;

    default: return false;
  }
  
  return true;
}
#endif // wxUSE_GUI
