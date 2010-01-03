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

wxExSTCEntryDialog::wxExSTCEntryDialog(wxWindow* parent,
  const wxString& caption,
  const wxString& text,
  const wxString& prompt,
  long button_style,
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

  // Currently we cannot find in the component,
  // so no wxExSTC::STC_MENU_FIND flag,
  // as this requires the component to be visible to public interface,
  // and the applications to forward the find events.
  m_STC = new wxExSTC(
    this, 
    text, 
    wxExSTC::STC_MENU_SIMPLE,
    wxID_ANY, 
    pos, 
    size);

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
  return m_STC->GetFileName().GetLexer().GetScintillaLexer();
}

const wxString wxExSTCEntryDialog::GetText() const 
{
  return m_STC->GetText();
}

wxCharBuffer wxExSTCEntryDialog::GetTextRaw() const 
{
  return m_STC->GetTextRaw();
}

void wxExSTCEntryDialog::SetLexer(const wxString& lexer) 
{
  m_STC->SetLexer(lexer);
}

void wxExSTCEntryDialog::SetText(const wxString& text, bool reset_lexer)
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

  if (reset_lexer)
  {
    if (!GetLexer().empty())
    {
      SetLexer(wxEmptyString);
    }
  }
}

#endif // wxUSE_GUI
