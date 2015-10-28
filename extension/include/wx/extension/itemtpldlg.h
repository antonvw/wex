////////////////////////////////////////////////////////////////////////////////
// Name:      itemtpldlg.h
// Purpose:   Declaration of wxExItemTemplateDialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <wx/aui/auibook.h>
#include <wx/bookctrl.h> 
#include <wx/choicebk.h>
#include <wx/filepicker.h>
#include <wx/imaglist.h>
#include <wx/listbook.h>
#include <wx/persist/treebook.h>
#include <wx/simplebook.h>
#include <wx/stc/stc.h>
#include <wx/tglbtn.h> // for wxEVT_TOGGLEBUTTON
#include <wx/toolbook.h>
#include <wx/extension/dialog.h>
#include <wx/extension/frame.h>

#if wxUSE_GUI
/// Offers a dialog template to set several items.
/// If you only specify a wxCANCEL button, the dialog is readonly.
/// When pressing the:
/// - wxAPPLY button
/// - wxOK, wxCANCEL button for a modeless dialog
/// - a ITEM_BUTTON
/// - a ITEM_COMBOBOXDIR
/// the method wxExFrame::OnCommandItemDialog is invoked.
template <class T> class WXDLLIMPEXP_BASE wxExItemTemplateDialog: public wxExDialog
{
public:
  /// Supported notebooks.
  enum
  {
    ITEM_AUINOTEBOOK, ///< a aui notebook
    ITEM_CHOICEBOOK,  ///< a choice book
    ITEM_LISTBOOK,    ///< a list book
    ITEM_NOTEBOOK,    ///< a traditional notebook
    ITEM_SIMPLEBOOK,  ///< a simple notebook
    ITEM_TOOLBOOK,    ///< a tool book
    ITEM_TREEBOOK,    ///< a tree book
  };

  /// Constructor.
  wxExItemTemplateDialog(
    /// parent
    wxWindow* parent,
    /// vector with items 
    const std::vector< T >& v,
    /// title
    const wxString& title = _("Options"),
    /// number of rows
    int rows = 0,
    /// number of columns
    int cols = 1,
    /// dialog flags for buttons
    long flags = wxOK | wxCANCEL,
    /// the window id
    wxWindowID id = wxID_ANY,
    /// bookctrl style, only used if you specified pages for your items
    int bookctrl_style = ITEM_AUINOTEBOOK,
    /// image list to be used by notebook (required for a tool book)
    wxImageList* imageList = NULL,
    /// position
    const wxPoint& pos = wxDefaultPosition,
    /// size
    const wxSize& size = wxDefaultSize, 
    /// dialog style
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
    /// name
    const wxString& name = "wxExItemTemplateDialog")
  : wxExDialog(parent, title, flags, id, pos, size, style, name)
  , m_ForceCheckBoxChecked(false)
  , m_Page(wxEmptyString)
  , m_Items(v) {
    Layout(rows, cols, bookctrl_style, imageList);
    Bind(wxEVT_BUTTON, &wxExItemTemplateDialog::OnCommand, this, wxID_APPLY);
    Bind(wxEVT_BUTTON, &wxExItemTemplateDialog::OnCommand, this, wxID_CANCEL);
    Bind(wxEVT_BUTTON, &wxExItemTemplateDialog::OnCommand, this, wxID_CLOSE);
    Bind(wxEVT_BUTTON, &wxExItemTemplateDialog::OnCommand, this, wxID_OK);
    Bind(wxEVT_UPDATE_UI, &wxExItemTemplateDialog::OnUpdateUI, this, wxID_APPLY);
    Bind(wxEVT_UPDATE_UI, &wxExItemTemplateDialog::OnUpdateUI, this, wxID_OK);};

  /// If you specified some checkboxes, calling this method
  /// requires that one of them should be checked for the OK button
  /// to be enabled.
  void ForceCheckBoxChecked(
    /// specify the (part of) the name of the checkbox
    const wxString& contains = wxEmptyString,
    /// specify on which page
    const wxString& page = wxEmptyString) {
    m_ForceCheckBoxChecked = true;
    m_Contains = contains;
    m_Page = page;};
  
  /// Returns the (first) item that has specified label,
  /// or empty item if item does not exist.
  const T GetItem(const wxString& label) const {
    for (const auto& it : m_Items)
    {
      if (it.GetLabel() == label)
      {
        return it;
      }
    };
    return T();};
  /// Return the item actual value for specified label, or 
  /// IsNull value if item does not exist.
  const wxAny GetItemValue(const wxString& label) const {
    return GetItem(label).GetValue();};
protected:
  const std::vector< T > & GetItems() const {return m_Items;};
  
  void OnCommand(wxCommandEvent& event) {
    if (  event.GetId() == wxID_APPLY ||
        ((event.GetId() == wxID_OK ||
          event.GetId() == wxID_CANCEL) && !IsModal()))
    {
      Click(event);
    }
    event.Skip();};
  
  void OnUpdateUI(wxUpdateUIEvent& event) {
    bool one_checkbox_checked = false;
    for (const auto& it : m_Items)
    {
      switch (it.GetType())
      {
      case ITEM_CHECKBOX:
        if (m_ForceCheckBoxChecked)
        {
          wxCheckBox* cb = (wxCheckBox*)it.GetWindow();
          if (it.GetLabel().Lower().Contains(m_Contains.Lower()) && 
              cb->IsChecked() &&
              it.GetPage() == m_Page)
          {
            one_checkbox_checked = true;
          }
        }
        break;

      case ITEM_CHECKLISTBOX_NONAME:
        if (m_ForceCheckBoxChecked)
        {
          wxCheckListBox* clb = (wxCheckListBox*)it.GetWindow();
          for (
            size_t i = 0;
            i < clb->GetCount();
            i++)
          {
            if (clb->GetString(i).Lower().Contains(m_Contains.Lower()) && 
                clb->IsChecked(i) &&
                it.GetPage() == m_Page)
            {
              one_checkbox_checked = true;
            }
          }
        }
        break;

      case ITEM_COMBOBOX:
      case ITEM_COMBOBOXDIR:
        {
        wxComboBox* cb = (wxComboBox*)it.GetWindow();
        if (it.GetIsRequired())
        {
          if (cb->GetValue().empty())
          {
            event.Enable(false);
            return;
          }
        }
        }
        break;

      case ITEM_INT:
      case ITEM_STRING:
        {
        wxTextCtrl* tc = (wxTextCtrl*)it.GetWindow();
        if (it.GetIsRequired())
        {
          if (tc->GetValue().empty())
          {
            event.Enable(false);
            return;
          }
        }
        }
        break;

      case ITEM_DIRPICKERCTRL:
        {
        wxDirPickerCtrl* pc = (wxDirPickerCtrl*)it.GetWindow();
        if (it.GetIsRequired())
        {
          if (pc->GetPath().empty())
          {
            event.Enable(false);
            return;
          }
        }
        }
        break;

      case ITEM_FILEPICKERCTRL:
        {
        wxFilePickerCtrl* pc = (wxFilePickerCtrl*)it.GetWindow();
        if (it.GetIsRequired())
        {
          if (pc->GetPath().empty())
          {
            event.Enable(false);
            return;
          }
        }
        }
        break;

      default: ; // do nothing
      }
    }
    event.Enable(m_ForceCheckBoxChecked ? one_checkbox_checked: true);};
private:
  void Click(const wxCommandEvent& event) const {
    wxExFrame* frame = wxDynamicCast(wxTheApp->GetTopWindow(), wxExFrame);
    if (frame != NULL)
    {
      frame->OnCommandItemDialog(GetId(), event);
    }};
  
  void Layout(int rows, int cols, int notebook_style, wxImageList* imageList) {
    wxBookCtrlBase* bookctrl = NULL;
    if (!m_Items.empty() && !m_Items.begin()->GetPage().empty())
    {
      switch (notebook_style)
      {
        case ITEM_AUINOTEBOOK: bookctrl = new wxAuiNotebook(this); break;
        case ITEM_CHOICEBOOK: bookctrl = new wxChoicebook(this, wxID_ANY); break;
        case ITEM_LISTBOOK: bookctrl = new wxListbook(this, wxID_ANY); break;
        case ITEM_NOTEBOOK: bookctrl = new wxNotebook(this, wxID_ANY); break;
        case ITEM_SIMPLEBOOK: bookctrl = new wxSimplebook(this, wxID_ANY); break;
        case ITEM_TREEBOOK: bookctrl = new wxTreebook(this, wxID_ANY); break;
        case ITEM_TOOLBOOK:
          bookctrl = new wxToolbook(this, wxID_ANY);
          if (imageList == NULL)
          {
            wxLogError("toolbook requires image list");
            return;
          }
          break;
    
        default: wxLogError("unknown bookctrl style");  
      }
      if (bookctrl != NULL)
      {
        bookctrl->SetImageList(imageList);
      }
    }
    bool first_time = true;
    wxString previous_page = "XXXXXX";
    wxFlexGridSizer* previous_item_sizer = NULL;
    wxFlexGridSizer* sizer = NULL;
    int previous_item_type = -1;
    for (auto& it : m_Items)
    {
      if (it.GetType() == ITEM_EMPTY) continue; //skip
      if (first_time ||
         (it.GetPage() != previous_page && !it.GetPage().empty()))
      {
        first_time = false;
        if (bookctrl != NULL)
        {
          // Finish the current page.
          if (bookctrl->GetCurrentPage() != NULL)
          {
            bookctrl->GetCurrentPage()->SetSizer(sizer);
          }
          // And make a new one.
          int imageId = wxWithImages::NO_IMAGE;
          if (imageList != NULL)
          {
            if ((int)bookctrl->GetPageCount() < imageList->GetImageCount())
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
            it.GetPage(), 
            true, // select
            imageId); 
        }
        const int use_cols = (it.GetColumns() != -1 ? it.GetColumns(): cols);
        sizer = (rows != 0 ? 
          new wxFlexGridSizer(rows, use_cols, 0, 0):
          new wxFlexGridSizer(use_cols));
        for (int i = 0; i < use_cols; i++)
        {
          sizer->AddGrowableCol(i);
        }
      }
      wxFlexGridSizer* use_item_sizer = (
        it.GetType() == previous_item_type && it.GetPage() == previous_page ? previous_item_sizer: 
        NULL);
      // Layout the item.
      previous_item_sizer = it.Layout(
        (bookctrl != NULL ? bookctrl->GetCurrentPage(): this), 
        sizer, 
        GetButtonFlags() == wxCANCEL,
        use_item_sizer);
      previous_item_type = it.GetType();
      previous_page = it.GetPage();
      if (sizer != NULL &&
          sizer->GetEffectiveRowsCount() >= 1 &&
         !sizer->IsRowGrowable(sizer->GetEffectiveRowsCount() - 1) &&
          it.IsRowGrowable())
      {
        sizer->AddGrowableRow(sizer->GetEffectiveRowsCount() - 1);
      }
      switch (it.GetType())
      {
        case ITEM_BUTTON:
        case ITEM_COMMAND_LINK_BUTTON:
          Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& event) {
            Click(event);}, it.GetWindow()->GetId());
          break;
        case ITEM_COMBOBOXDIR:
          Bind(wxEVT_COMMAND_BUTTON_CLICKED, [=](wxCommandEvent& event) {
            wxComboBox* browse = (wxComboBox*)it.GetWindow();
            wxDirDialog dir_dlg(
              this,
              _(wxDirSelectorPromptStr),
              browse->GetValue(),
              wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
            if (dir_dlg.ShowModal() == wxID_OK)
            {
              const wxString value = dir_dlg.GetPath();
              const int item = browse->FindString(value);
              browse->SetSelection(item == wxNOT_FOUND ? browse->Append(value): item);
            }}, it.GetWindow()->GetId());
          break;
        case ITEM_TOGGLEBUTTON:
          Bind(wxEVT_TOGGLEBUTTON, [=](wxCommandEvent& event) {
            Click(event);}, it.GetWindow()->GetId());
          break;
      }
    }
    if (bookctrl != NULL)
    {
      if (bookctrl->GetCurrentPage() != NULL)
      {
        bookctrl->GetCurrentPage()->SetSizer(sizer);
      }
      bookctrl->SetName("book" + GetName());
      if (!wxPersistenceManager::Get().RegisterAndRestore(bookctrl))
      {
        // nothing was restored, so choose the default page ourselves
        bookctrl->SetSelection(0);
      }
      AddUserSizer(bookctrl);
    }
    else if (sizer != NULL)
    {
      AddUserSizer(sizer);
    }
    LayoutSizers(bookctrl == NULL); // add separator line if no bookctrl
  };

  std::vector< T > m_Items;
  bool m_ForceCheckBoxChecked;
  wxString m_Contains;
  wxString m_Page;
};
#endif // wxUSE_GUI
