////////////////////////////////////////////////////////////////////////////////
// Name:      stcdlg.cpp
// Purpose:   Implementation of class wxExSTCEntryDialog
// Author:    Anton van Wezenbeek
// Created:   2009-11-18
// RCS-ID:    $Id$
// Copyright: (c) 2009 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/stcdlg.h>
#include <wx/extension/stc.h>

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
  wxWindowID id,
  long style)
  : wxExDialog(parent, caption, button_style, id, style)
{
  if (!prompt.empty())
  {
    AddUserSizer(CreateTextSizer(prompt), wxSizerFlags().Center());
  }

  m_STC = new wxExSTC(
    this, 
    wxExSTC::STC_MENU_SIMPLE | 
      wxExSTC::STC_MENU_FIND | 
      wxExSTC::STC_MENU_REPLACE,
    wxID_ANY, 
    wxDefaultPosition, 
    wxDefaultSize);

  m_STC->SetText(text);

  // Override defaults from config.
  m_STC->SetEdgeMode(wxSTC_EDGE_NONE);
  m_STC->ResetMargins();
  m_STC->SetViewEOL(false);
  m_STC->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);

  if ((button_style & wxCANCEL) == 0 &&
      (button_style & wxNO) == 0)
  {
    // You did not specify one of these buttons,
    // so you cannot cancel the operation.
    // Therefore make the component readonly.
    m_STC->SetReadOnly(true);
  }

  AddUserSizer(m_STC);

  LayoutSizers();
}

const wxString wxExSTCEntryDialog::GetLexer() const
{
  return m_STC->GetLexer().GetScintillaLexer();
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

void wxExSTCEntryDialog::SetLexer(const wxString& lexer) 
{
  m_STC->SetLexer(lexer);
}

void wxExSTCEntryDialog::SetText(const wxString& text)
{
  const bool readonly = m_STC->GetReadOnly();

  if (readonly)
  {
    m_STC->SetReadOnly(false);
  }

  m_STC->SetText(text);

  if (readonly)
  {
    m_STC->SetReadOnly(true);
  }
}

#endif // wxUSE_GUI
