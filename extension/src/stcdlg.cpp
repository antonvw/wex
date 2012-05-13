////////////////////////////////////////////////////////////////////////////////
// Name:      stcdlg.cpp
// Purpose:   Implementation of class wxExSTCEntryDialog
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/persist/toplevel.h>
#include <wx/extension/stcdlg.h>
#include <wx/extension/shell.h>

#if wxUSE_GUI

BEGIN_EVENT_TABLE(wxExSTCEntryDialog, wxExDialog)
  EVT_MENU(wxID_FIND, wxExSTCEntryDialog::OnCommand)
  EVT_MENU(wxID_REPLACE, wxExSTCEntryDialog::OnCommand)
END_EVENT_TABLE()

wxExSTCEntryDialog::wxExSTCEntryDialog(wxWindow* parent,
  const wxString& caption,
  const wxString& text,
  const wxString& prompt,
  long button_style,
  bool use_shell,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size, 
  long style,
  const wxString& name)
  : wxExDialog(parent, caption, button_style, id, pos, size, style, name)
{
  if (!prompt.empty())
  {
    AddUserSizer(CreateTextSizer(prompt), wxSizerFlags().Center());
  }

  wxPersistentRegisterAndRestore(this);

  m_STC =  (use_shell ?
    new wxExSTCShell(this, text):
    new wxExSTC(this, text));
  
  m_STC->SetEdgeMode(wxSTC_EDGE_NONE);
  m_STC->ResetMargins();
  m_STC->SetViewEOL(false);
  m_STC->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);
  
  AddUserSizer(m_STC);

  LayoutSizers();
}

const wxExLexer* wxExSTCEntryDialog::GetLexer() const
{
  return &m_STC->GetLexer();
}

wxExSTC* wxExSTCEntryDialog::GetSTC()
{
  return m_STC;
}

wxExSTCShell* wxExSTCEntryDialog::GetSTCShell()
{
  return (wxExSTCShell *)m_STC;
}

const wxString wxExSTCEntryDialog::GetText() const 
{
  return m_STC->GetText();
}

const wxCharBuffer wxExSTCEntryDialog::GetTextRaw() const 
{
  return m_STC->GetTextRaw();
}

void wxExSTCEntryDialog::OnCommand(wxCommandEvent& command)
{
  switch (command.GetId())
  {
    // Without these, events are not handled by the frame.
    case wxID_FIND: 
    case wxID_REPLACE: 
      wxPostEvent(wxTheApp->GetTopWindow(), command);
      break;

    default: wxFAIL;
  }
}

bool wxExSTCEntryDialog::SetLexer(const wxString& lexer) 
{
  return m_STC->SetLexer(lexer);
}

#endif // wxUSE_GUI
