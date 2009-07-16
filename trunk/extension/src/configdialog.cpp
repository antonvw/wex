/******************************************************************************\
* File:          configdialog.cpp
* Purpose:       Implementation of wxWidgets config dialog classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/aui/auibook.h>
#include <wx/clrpicker.h> // for wxColourPickerWidget
#include <wx/filepicker.h>
#include <wx/fontpicker.h>
#include <wx/spinctrl.h>
#include <wx/extension/configdialog.h>
#include <wx/extension/stc.h> // for PathListInit
#include <wx/extension/util.h>

using namespace std;

#if wxUSE_GUI
const int width = 200;
const int width_numeric = 75;

enum
{
  ID_BROWSE_FOLDER = 100,
};

BEGIN_EVENT_TABLE(wxExConfigDialog, wxExDialog)
  EVT_BUTTON(wxID_APPLY, wxExConfigDialog::OnCommand)
  EVT_BUTTON(wxID_OK, wxExConfigDialog::OnCommand)
  EVT_BUTTON(ID_BROWSE_FOLDER, wxExConfigDialog::OnCommand)
  EVT_UPDATE_UI(wxID_OK, wxExConfigDialog::OnUpdateUI)
END_EVENT_TABLE()

// wxPropertySheetDialog has been tried as well,
// then you always have a notebook, and apply button is not supported.
wxExConfigDialog::wxExConfigDialog(wxWindow* parent,
  wxExConfig* config,
  vector<wxExConfigItem> v,
  const wxString& title,
  const wxString& configGroup,
  int rows,
  int cols,
  long flags,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxExDialog(parent, title, flags, id, pos, size, style)
  , m_Config(config)
  , m_ConfigGroup(configGroup)
{
  bool first_time = true;
  wxFlexGridSizer* sizer = NULL;
  wxFlexGridSizer* notebook_sizer = NULL;
  wxAuiNotebook* notebook = NULL;
  wxString previous_page = "XXXXXX";
  wxPanel* page_panel = NULL;

  for (
    vector<wxExConfigItem>::iterator it = v.begin();
    it != v.end();
    ++it)
  {
    // Check if we need a notebook.
    if (it == v.begin() && !it->m_Page.empty())
    {
      // Do not give it a close button.
      notebook = new wxAuiNotebook(this,
        wxID_ANY,
        pos,
        size,
        wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_TAB_SPLIT);

      notebook_sizer = new wxFlexGridSizer(1);
      notebook_sizer->AddGrowableCol(0);
      notebook_sizer->Add(notebook, wxSizerFlags().Expand().Center());
      notebook_sizer->AddGrowableRow(0);
      notebook_sizer->SetMinSize(size);
    }

    if (first_time ||
        (it->m_Page != previous_page &&
         it->m_Page != wxEmptyString))
    {
      first_time = false;

      if (notebook != NULL && it->m_Type != CONFIG_SPACER)
      {
        // Finish the current page.
        if (sizer != NULL)
        {
          page_panel->SetSizerAndFit(sizer);
        }

        // And make a new one.
        page_panel = new wxPanel(notebook);
        notebook->AddPage(page_panel, it->m_Page);
      }

      previous_page = it->m_Page;

      if (rows != 0)
        sizer = new wxFlexGridSizer(rows, cols, 0, 0);
      else
        sizer = new wxFlexGridSizer(cols);

      if (cols == 2)
      {
        sizer->AddGrowableCol(1);
      }
      else
      {
        for (int i = 0; i < cols; i++)
        {
          sizer->AddGrowableCol(i);
        }
      }
    }

    wxWindow* parent =
      (page_panel != NULL ? (wxWindow*)page_panel: this);

    wxControl* control = NULL;

    switch (it->m_Type)
    {
    case CONFIG_CHECKBOX:
      control = AddCheckBox(parent, sizer, it->m_Name);
      break;

    case CONFIG_CHECKLISTBOX:
      control = AddCheckListBox(parent, sizer, it->m_Name, it->m_Choices);
      break;

    case CONFIG_CHECKLISTBOX_NONAME:
      control = AddCheckListBoxNoName(parent, sizer, it->m_ChoicesBool);
      break;

    case CONFIG_COLOUR:
      control = AddColourButton(parent, sizer, it->m_Name);
      break;

    case CONFIG_COMBOBOX:
      control = AddComboBox(parent, sizer, it->m_Name);
      break;

    case CONFIG_COMBOBOXDIR:
      control = AddComboBoxDir(parent, sizer, it->m_Name);
      break;

    case CONFIG_DIRPICKERCTRL:
      control = AddDirPickerCtrl(parent, sizer, it->m_Name);
      break;

    case CONFIG_FILEPICKERCTRL:
      control = AddFilePickerCtrl(parent, sizer, it->m_Name);
      break;

    case CONFIG_FONTPICKERCTRL:
      control = AddFontPickerCtrlCtrl(parent, sizer, it->m_Name);
      break;

    case CONFIG_INT:
      control = AddTextCtrl(parent, sizer, it->m_Name, true);
      break;

    case CONFIG_RADIOBOX:
      control = AddRadioBox(parent, sizer, it->m_Name, it->m_Choices);
      break;

    case CONFIG_SPACER:
      sizer->AddSpacer(wxSizerFlags::GetDefaultBorder());
      break;

    case CONFIG_SPINCTRL:
      control = AddSpinCtrl(parent, sizer, it->m_Name, it->m_Min, it->m_Max);
      break;

    case CONFIG_SPINCTRL_DOUBLE:
      control = AddSpinCtrlDouble(parent, sizer, it->m_Name, it->m_MinDouble, it->m_MaxDouble, it->m_Inc);
      break;

    case CONFIG_STRING:
      control = AddTextCtrl(parent, sizer, it->m_Name, false, it->m_Style);
      break;

    default:
      wxFAIL;
      return;
      break;
    }

    if (sizer != NULL)
    {
      if ( sizer->GetRows() > 0 && 
          !sizer->IsRowGrowable(sizer->GetRows() - 1))
      {
        sizer->AddGrowableRow(sizer->GetRows() - 1);
      }
    }

    if (control != NULL)
    {
      control->SetName(it->m_Name);
      it->m_Control = control;
    }

    m_ConfigItems.push_back(*it);
  }

  if (page_panel != NULL && notebook_sizer != NULL && sizer != NULL)
  {
    page_panel->SetSizer(sizer);

    AddUserSizer(notebook_sizer);

    SetMinSize(size);

    SendSizeEvent();
  }
  else
  {
    AddUserSizer(sizer);
  }

  BuildSizers();
}

wxControl* wxExConfigDialog::Add(
  wxSizer* sizer,
  wxWindow* parent,
  wxControl* control,
  const wxString& text,
  bool expand)
{
  wxSizerFlags flags;
  flags.Border();

  sizer->Add(new wxStaticText(parent, wxID_ANY, text), flags.Right());
  sizer->Add(control, (expand ? flags.Left().Expand(): flags.Left()));

  return control;
}

wxControl* wxExConfigDialog::AddCheckBox(wxWindow* parent,
  wxSizer* sizer, const wxString& text)
{
  wxCheckBox* checkbox = new wxCheckBox(parent,
    wxID_ANY,
    text,
    wxDefaultPosition,
    wxSize(125, wxDefaultCoord));

  checkbox->SetValue(m_Config->GetBool(m_ConfigGroup + text, false));

  wxSizerFlags flags;
  flags.Expand().Left().Border();
  sizer->Add(checkbox, flags);

  return checkbox;
}

wxControl* wxExConfigDialog::AddCheckListBox(wxWindow* parent,
  wxSizer* sizer, const wxString& text, std::map<long, const wxString> & choices)
{
  wxArrayString arraychoices;

  for (
    std::map<long, const wxString>::const_iterator it = choices.begin();
    it != choices.end();
    ++it)
  {
    arraychoices.Add(it->second);
  }

  wxCheckListBox* box = new wxCheckListBox(parent,
    wxID_ANY, wxDefaultPosition, wxDefaultSize, arraychoices);

  const long value = m_Config->Get(m_ConfigGroup + text, 0);

  int item = 0;
  for (
    std::map<long, const wxString>::const_iterator it = choices.begin();
    it != choices.end();
    ++it)
  {
    if (value & it->first)
    {
      box->Check(item);
    }

    item++;
  }

  return Add(sizer, parent, box, text + ":");
}

wxControl* wxExConfigDialog::AddCheckListBoxNoName(wxWindow* parent,
  wxSizer* sizer, std::set<wxString> & choices)
{
  wxArrayString arraychoices;

  for (
    std::set<wxString>::const_iterator it = choices.begin();
    it != choices.end();
    ++it)
  {
    arraychoices.Add(*it);
  }

  wxCheckListBox* box = new wxCheckListBox(parent,
    wxID_ANY, wxDefaultPosition, wxDefaultSize, arraychoices);

  int item = 0;
  for (
    std::set<wxString>::const_iterator it = choices.begin();
    it != choices.end();
    ++it)
  {
    // Special cases, should be taken from the find replace data.
    if (*it == _("Match whole word"))
    {
      if (m_Config->GetFindReplaceData()->MatchWord())
      {
        box->Check(item);
      }
    }
    else if (*it == _("Match case"))
    {
      if (m_Config->GetFindReplaceData()->MatchCase())
      {
        box->Check(item);
      }
    }
    else if (*it == _("Regular expression"))
    {
      if (m_Config->GetFindReplaceData()->IsRegularExpression())
      {
        box->Check(item);
      }
    }
    else if (m_Config->GetBool(m_ConfigGroup + *it))
    {
      box->Check(item);
    }

    item++;
  }

  wxSizerFlags flags;
  flags.Expand().Left().Border();
  sizer->Add(box, flags);

  return box;
}

wxControl* wxExConfigDialog::AddColourButton(wxWindow* parent,
  wxSizer* sizer, const wxString& text)
{
  return Add(
    sizer,
    parent,
    new wxColourPickerWidget(parent,
      wxID_ANY,
      m_Config->Get(m_ConfigGroup + text, wxExColourToLong(*wxWHITE))),
    text + ":",
    false); // do not expand
}

wxControl* wxExConfigDialog::AddComboBox(wxWindow* parent,
  wxSizer* sizer, const wxString& text)
{
  wxComboBox* cb = new wxComboBox(parent, wxID_ANY);
  wxExComboBoxFromString(
    cb,
    m_Config->Get(
      m_ConfigGroup + text,
      wxEmptyString,
      '@')); // no delimiter!

  if (text == _("Find what"))
  {
    const wxString frd = m_Config->GetFindReplaceData()->GetFindString();
    if (!frd.empty())
    {
      if (cb->FindString(frd) == wxNOT_FOUND)
      {
        cb->Append(frd);
      }

      cb->SetValue(frd);
    }
  }
  else if (text == _("Replace with"))
  {
    const wxString frd = m_Config->GetFindReplaceData()->GetReplaceString();
    if (!frd.empty())
    {
      if (cb->FindString(frd) == wxNOT_FOUND)
      {
        cb->Append(frd);
      }

      cb->SetValue(frd);
    }
  }

  return Add(sizer, parent, cb, text + ":");
}

wxControl* wxExConfigDialog::AddComboBoxDir(wxWindow* parent,
  wxSizer* sizer, const wxString& text)
{
  wxComboBox* cb = new wxComboBox(parent, ID_BROWSE_FOLDER + 1);
  wxExComboBoxFromString(
    cb,
    m_Config->Get(
      m_ConfigGroup + text,
      wxEmptyString,
      '@')); // no delimiter!

  wxSizerFlags flag;

  wxFlexGridSizer* browse = new wxFlexGridSizer(2, 0, 0);
  browse->AddGrowableCol(0);
  browse->Add(cb, flag.Expand());

  // Tried to use a wxDirPickerCtrl here, is nice,
  // but does not use a combobox for keeping old values, so this is better.
  // And the text box that is used is not resizable as well.
  browse->Add(
    new wxButton(
      this,
      ID_BROWSE_FOLDER,
      "...",
      wxDefaultPosition,
      wxDefaultSize,
      wxBU_EXACTFIT),
    flag.Center().Border(wxLEFT));

  sizer->Add(new wxStaticText(this, wxID_ANY, text + ":"), flag.Right().Border());
  sizer->Add(browse, flag.Center().Border());

  return cb;
}

wxControl* wxExConfigDialog::AddDirPickerCtrl(wxWindow* parent,
  wxSizer* sizer, const wxString& text)
{
  wxDirPickerCtrl* pc = new wxDirPickerCtrl(parent,
    wxID_ANY,
    m_Config->Get(m_ConfigGroup + text),
    wxEmptyString,
    wxDefaultPosition,
    wxSize(width, wxDefaultCoord));

  // If only cancel button, make readonly.
  if (pc->GetTextCtrl() != NULL && GetFlags() == wxCANCEL)
  {
    pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
  }

  return Add(sizer, parent, pc, text + ":");
}

wxControl* wxExConfigDialog::AddFilePickerCtrl(wxWindow* parent,
  wxSizer* sizer, const wxString& text)
{
  wxFilePickerCtrl* pc = new wxFilePickerCtrl(parent,
    wxID_ANY,
    m_Config->Get(m_ConfigGroup + text),
    wxEmptyString,
    wxEmptyString,
    wxDefaultPosition,
    wxSize(width, wxDefaultCoord));

  // If only cancel button, make readonly.
  if (pc->GetTextCtrl() != NULL && GetFlags() == wxCANCEL)
  {
    pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
  }

  return Add(sizer, parent, pc, text + ":");
}

wxControl* wxExConfigDialog::AddFontPickerCtrlCtrl(wxWindow* parent,
  wxSizer* sizer, const wxString& text)
{
  wxFont font(
    m_Config->Get(m_ConfigGroup + text + "/Size", 10),
    wxFONTFAMILY_DEFAULT,
    wxFONTSTYLE_NORMAL,
    wxFONTWEIGHT_NORMAL,
    false,
    m_Config->Get(m_ConfigGroup + text + "/Name", "courier new"));

  wxFontPickerCtrl* pc = new wxFontPickerCtrl(parent,
    wxID_ANY,
    font,
    wxDefaultPosition,
    wxSize(width, wxDefaultCoord));

  // If only cancel button, make readonly.
  if (pc->GetTextCtrl() != NULL && GetFlags() == wxCANCEL)
  {
    pc->GetTextCtrl()->SetWindowStyleFlag(wxTE_READONLY);
  }

  return Add(sizer, parent, pc, text + ":");
}

wxControl* wxExConfigDialog::AddRadioBox(wxWindow* parent,
  wxSizer* sizer, const wxString& text, std::map<long, const wxString> & choices)
{
  wxArrayString arraychoices;

  for (
    std::map<long, const wxString>::const_iterator it = choices.begin();
    it != choices.end();
    ++it)
  {
    arraychoices.Add(it->second);
  }

  wxRadioBox* box = new wxRadioBox(parent,
    wxID_ANY, text, wxDefaultPosition, wxDefaultSize, arraychoices, 0, wxRA_SPECIFY_ROWS);

  box->SetStringSelection(choices[m_Config->Get(m_ConfigGroup + text, 0)]);

  wxSizerFlags flags;
  flags.Expand().Left().Border();
  sizer->Add(box, flags);

  return box;
}

wxControl* wxExConfigDialog::AddSpinCtrl(wxWindow* parent,
  wxSizer* sizer, const wxString& text, int min, int max)
{
  long style = wxSP_ARROW_KEYS;

  // If only cancel button, make readonly.
  if (GetFlags() == wxCANCEL)
  {
    style |= wxTE_READONLY;
  }

  wxSpinCtrl* spinctrl = new wxSpinCtrl(parent,
    wxID_ANY,
    wxEmptyString,
    wxDefaultPosition,
    wxSize(width_numeric, wxDefaultCoord),
    style,
    min,
    max,
    m_Config->Get(m_ConfigGroup + text, min));

  spinctrl->SetValue(m_Config->Get(m_ConfigGroup + text, min));

  return Add(sizer, parent, spinctrl, text + ":", false);
}

wxControl* wxExConfigDialog::AddSpinCtrlDouble(wxWindow* parent,
  wxSizer* sizer, const wxString& text, double min, double max, double inc)
{
  long style = wxSP_ARROW_KEYS;

  // If only cancel button, make readonly.
  if (GetFlags() == wxCANCEL)
  {
    style |= wxTE_READONLY;
  }

  wxSpinCtrlDouble* spinctrl = new wxSpinCtrlDouble(parent,
    wxID_ANY,
    wxEmptyString,
    wxDefaultPosition,
    wxSize(width_numeric, wxDefaultCoord),
    style,
    min,
    max,
    inc,
    m_Config->Get(m_ConfigGroup + text, min));

  spinctrl->SetValue(m_Config->Get(m_ConfigGroup + text, min));

  return Add(sizer, parent, spinctrl, text + ":", false);
}

wxControl* wxExConfigDialog::AddTextCtrl(wxWindow* parent,
  wxSizer* sizer, const wxString& text, bool is_numeric, long style)
{
  const wxString value =
    (!is_numeric ?
        m_Config->Get(m_ConfigGroup + text):
        wxString::Format("%ld", m_Config->Get(m_ConfigGroup + text, 0)));

  long actual_style = style;
  int actual_width = width;

  // Add alignment for numerics.
  if (is_numeric)
  {
    actual_style |= wxTE_RIGHT;
    actual_width = width_numeric;
  }

  // If only cancel button, make readonly.
  if (GetFlags() == wxCANCEL)
  {
    actual_style |= wxTE_READONLY;
  }

  wxTextCtrl* textctrl = new wxTextCtrl(parent,
    wxID_ANY,
    value,
    wxDefaultPosition,
    (style & wxTE_MULTILINE ?
       wxSize(actual_width, 200):
       wxSize(actual_width, wxDefaultCoord)),
    actual_style);

  if (is_numeric)
  {
    textctrl->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
  }

  return Add(sizer, parent, textctrl, text + ":", !is_numeric);
}

void wxExConfigDialog::OnCommand(wxCommandEvent& command)
{
  bool path_involved = false;

  if (command.GetId() == ID_BROWSE_FOLDER)
  {
    wxComboBox* cb = wxDynamicCast(
      FindWindowById(command.GetId() + 1, this), wxComboBox);

    wxASSERT(cb != NULL);

    wxDirDialog dir_dlg(
      this,
      _("Select Directory"),
      cb->GetValue(),
      wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

    if (dir_dlg.ShowModal() == wxID_OK)
    {
      cb->SetValue(dir_dlg.GetPath());
    }

    return;
  }

  // For rest of the buttons (wxID_OK, wxID_APPLY)
  // save to config.
  for (
    vector<wxExConfigItem>::const_iterator it = m_ConfigItems.begin();
    it != m_ConfigItems.end();
    ++it)
  {
    switch (it->m_Type)
    {
    case CONFIG_CHECKBOX:
      {
      wxCheckBox* cb = (wxCheckBox*)it->m_Control;
      m_Config->SetBool(m_ConfigGroup + cb->GetName(), cb->GetValue());
      }
      break;

    case CONFIG_CHECKLISTBOX:
      {
      wxCheckListBox* clb = (wxCheckListBox*)it->m_Control;

      long value = 0;
      int item = 0;

      for (
        std::map<long, const wxString>::const_iterator b = it->m_Choices.begin();
        b != it->m_Choices.end();
        ++b)
      {
        if (clb->IsChecked(item))
        {
          value |= b->first;
        }

        item++;
      }

      m_Config->Set(m_ConfigGroup + clb->GetName(), value);
      }
      break;

    case CONFIG_CHECKLISTBOX_NONAME:
      {
      wxCheckListBox* clb = (wxCheckListBox*)it->m_Control;

      int item = 0;

      for (
        std::set<wxString>::const_iterator b = it->m_ChoicesBool.begin();
        b != it->m_ChoicesBool.end();
        ++b)
      {
        // Special case, should be taken from find replace data.
        if (*b == _("Match whole word"))
        {
          m_Config->GetFindReplaceData()->SetMatchWord(clb->IsChecked(item));
        }
        else if (*b == _("Match case"))
        {
          m_Config->GetFindReplaceData()->SetMatchCase(clb->IsChecked(item));
        }
        else if (*b == _("Regular expression"))
        {
          m_Config->GetFindReplaceData()->SetIsRegularExpression(clb->IsChecked(item));
        }
        else
        {
          m_Config->SetBool(m_ConfigGroup + *b, clb->IsChecked(item));
        }

        item++;
      }

      }
      break;

    case CONFIG_COLOUR:
      {
      wxColourPickerWidget* gcb = (wxColourPickerWidget*)it->m_Control;
      m_Config->Set(
        m_ConfigGroup + gcb->GetName(),
        wxExColourToLong(gcb->GetColour()));
      }
      break;

    case CONFIG_COMBOBOX:
    case CONFIG_COMBOBOXDIR:
      {
      wxComboBox* cb = (wxComboBox*)it->m_Control;
      wxString text;
      if (wxExComboBoxToString(cb, text, ',', it->m_MaxItems))
      {
        m_Config->Set(m_ConfigGroup + cb->GetName(), text);

        if (cb->GetName() == _("Find what"))
        {
          // The Get gets text before the ','!
          m_Config->GetFindReplaceData()->SetFindString(m_Config->Get(_("Find what")));
        }
        else if (cb->GetName() == _("Replace with"))
        {
          // The Get gets text before the ','!
          m_Config->GetFindReplaceData()->SetReplaceString(m_Config->Get(_("Replace with")));
        }
      }
      }
      break;

    case CONFIG_DIRPICKERCTRL:
      {
      wxDirPickerCtrl* pc = (wxDirPickerCtrl*)it->m_Control;
      m_Config->Set(m_ConfigGroup + pc->GetName(), pc->GetPath());
      }
      break;

    case CONFIG_FILEPICKERCTRL:
      {
      wxFilePickerCtrl* pc = (wxFilePickerCtrl*)it->m_Control;
      m_Config->Set(m_ConfigGroup + pc->GetName(), pc->GetPath());
      }
      break;

    case CONFIG_FONTPICKERCTRL:
      {
      wxFontPickerCtrl* pc = (wxFontPickerCtrl*)it->m_Control;
      m_Config->Set(
        m_ConfigGroup + pc->GetName() + "/Size",
        pc->GetSelectedFont().GetPointSize());
      m_Config->Set(
        m_ConfigGroup + pc->GetName() + "/Name",
        pc->GetSelectedFont().GetFaceName());
      }
      break;

    case CONFIG_INT:
      {
      wxTextCtrl* tc = (wxTextCtrl*)it->m_Control;
      m_Config->Set(m_ConfigGroup + tc->GetName(), atol(tc->GetValue().c_str()));
      }
      break;

    case CONFIG_RADIOBOX:
      {
      wxRadioBox* rb = (wxRadioBox*)it->m_Control;

      for (
        std::map<long, const wxString>::const_iterator b = it->m_Choices.begin();
        b != it->m_Choices.end();
        ++b)
      {
        if (b->second == rb->GetStringSelection())
        {
          m_Config->Set(m_ConfigGroup + rb->GetName(), b->first);
        }
      }
      }
      break;

    case CONFIG_SPACER:
      break;

    case CONFIG_STRING:
      {
      wxTextCtrl* tc = (wxTextCtrl*)it->m_Control;
      m_Config->Set(m_ConfigGroup + tc->GetName(), tc->GetValue());
      if (tc->GetName() == _("Include directory"))
      {
        path_involved = true;
      }
      }
      break;

    case CONFIG_SPINCTRL:
      {
      wxSpinCtrl* sc = (wxSpinCtrl*)it->m_Control;
      m_Config->Set(m_ConfigGroup + sc->GetName(), sc->GetValue());
      }
      break;

    case CONFIG_SPINCTRL_DOUBLE:
      {
      wxSpinCtrlDouble* sc = (wxSpinCtrlDouble*)it->m_Control;
      m_Config->Set(m_ConfigGroup + sc->GetName(), sc->GetValue());
      }
      break;

    default:
      wxFAIL;
      break;
    }
  }

  if ( command.GetId() == wxID_APPLY ||
      (command.GetId() == wxID_OK && !IsModal()))
  {
    if (wxTheApp != NULL)
    {
      wxWindow* window = wxTheApp->GetTopWindow();
      wxExFrame* frame = wxDynamicCast(window, wxExFrame);

      if (frame != NULL)
      {
        frame->ConfigDialogApplied(GetId());

        if (path_involved)
        {
          wxExSTC::PathListInit();
        }
      }
    }
  }

  if (command.GetId() == wxID_OK)
  {
    if (path_involved)
    {
      wxExSTC::PathListInit();
    }

    EndDialog(wxID_OK);
  }
}

void wxExConfigDialog::OnUpdateUI(wxUpdateUIEvent& event)
{
  for (
    vector<wxExConfigItem>::const_iterator it = m_ConfigItems.begin();
    it != m_ConfigItems.end();
    ++it)
  {
    switch (it->m_Type)
    {
    case CONFIG_COMBOBOX:
    case CONFIG_COMBOBOXDIR:
      {
      wxComboBox* cb = (wxComboBox*)it->m_Control;
      if (it->m_IsRequired)
      {
        if (cb->GetValue().empty())
        {
          event.Enable(false);
          return;
        }
      }
      }
      break;

    case CONFIG_INT:
    case CONFIG_STRING:
      {
      wxTextCtrl* tc = (wxTextCtrl*)it->m_Control;
      if (it->m_IsRequired)
      {
        if (tc->GetValue().empty())
        {
          event.Enable(false);
          return;
        }
      }
      }
      break;

    case CONFIG_DIRPICKERCTRL:
      {
      wxDirPickerCtrl* pc = (wxDirPickerCtrl*)it->m_Control;
      if (it->m_IsRequired)
      {
        if (pc->GetPath().empty())
        {
          event.Enable(false);
          return;
        }
      }
      }
      break;

    case CONFIG_FILEPICKERCTRL:
      {
      wxFilePickerCtrl* pc = (wxFilePickerCtrl*)it->m_Control;
      if (it->m_IsRequired)
      {
        if (pc->GetPath().empty())
        {
          event.Enable(false);
          return;
        }
      }
      }
      break;
    }
  }

  event.Enable(true);
}
#endif // wxUSE_GUI
