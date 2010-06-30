/******************************************************************************\
* File:          stcfile.cpp
* Purpose:       Implementation of class wxExSTCFile
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/stcfile.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/filename.h>
#include <wx/extension/frame.h>
#include <wx/extension/lexers.h>
#include <wx/extension/log.h>
#include <wx/extension/stc.h>

#if wxUSE_GUI
const int SCI_ADDTEXT = 2001;
const int SCI_APPENDTEXT = 2282;

wxExSTCFile::wxExSTCFile(wxExSTC* stc)
  : m_STC(stc)
  , m_PreviousLength(0)
{
}

void wxExSTCFile::DoFileLoad(bool synced)
{
  if (GetContentsChanged())
  {
    wxExFileDialog dlg(m_STC, this);
    if (dlg.ShowModalIfChanged() == wxID_CANCEL) return;
  }

  // Synchronizing by appending only new data only works for log files.
  // Other kind of files might get new data anywhere inside the file,
  // we cannot sync that by keeping pos. Also only do it for reasonably large files,
  // so small log files are synced always (e.g. COM LIB report.log).
  const bool log_sync =
    synced &&
    GetFileName().GetExt().CmpNoCase("log") == 0 &&
    m_STC->GetTextLength() > 1024;

  // Be sure we can add text.
  m_STC->SetReadOnly(false);

  ReadFromFile(log_sync);

  if (!(m_STC->GetFlags() & wxExSTC::STC_WIN_HEX))
  {
    m_STC->SetLexer(GetFileName().GetLexer().GetScintillaLexer());

    if (m_STC->GetLexer().GetScintillaLexer() == "po")
    {
      m_STC->AddBasePathToPathList();
    }
  }

  if (m_STC->GetFlags() & m_STC->STC_WIN_READ_ONLY ||
      GetFileName().GetStat().IsReadOnly() ||
      // At this moment we do not allow to write in hex mode.
      m_STC->GetFlags() & m_STC->STC_WIN_HEX)
  {
    m_STC->SetReadOnly(true);
  }

  m_STC->EmptyUndoBuffer();

  if (!synced)
  {
    wxExLog::Get()->Log(_("Opened") + ": " + GetFileName().GetFullPath());
    m_STC->PropertiesMessage();
  }
  else
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(GetFileName(), wxExFrame::STAT_SYNC);
    m_STC->UpdateStatusBar("PaneLines");
#endif
  }

  // No edges for log files.
  if (GetFileName().GetExt() == "log")
  {
    m_STC->SetEdgeMode(wxSTC_EDGE_NONE);
  }

  if (GetFileName() == wxExLog::Get()->GetFileName())
  {
    m_STC->DocumentEnd();
  }
}

void wxExSTCFile::DoFileNew()
{
  m_STC->SetName(GetFileName().GetFullPath());

  m_STC->PropertiesMessage();

  m_STC->ClearDocument();

  m_STC->SetLexer(GetFileName().GetLexer().GetScintillaLexer());
}

void wxExSTCFile::DoFileSave(bool save_as)
{
  const wxCharBuffer& buffer = m_STC->GetTextRaw(); 
  Write(buffer.data(), buffer.length());

  if (save_as)
  {
    m_STC->SetName(GetFileName().GetFullPath());
    m_STC->SetLexer(GetFileName().GetLexer().GetScintillaLexer());
  }
  
  if (wxExLexers::Get()->MarkerIsLoaded(m_STC->GetMarkerChange()))
  {
    m_STC->MarkerDeleteAll(m_STC->GetMarkerChange().GetNo());
  }

  const wxString msg = _("Saved") + ": " + GetFileName().GetFullPath();
  wxExLog::Get()->Log(msg);
  
#if wxUSE_STATUSBAR
  wxExFrame::StatusText(msg);
#endif
}

bool wxExSTCFile::GetContentsChanged() const 
{
  return m_STC->GetModify();
}

void wxExSTCFile::ReadFromFile(bool get_only_new_data)
{
  const bool pos_at_end = (m_STC->GetCurrentPos() >= m_STC->GetTextLength() - 1);

  int startPos, endPos;
  m_STC->GetSelection(&startPos, &endPos);

  wxFileOffset offset = 0;

  if (m_PreviousLength < Length() && get_only_new_data)
  {
    offset = m_PreviousLength;
  }

  if (offset == 0)
  {
    m_STC->ClearDocument();
  }

  m_PreviousLength = Length();

  const wxCharBuffer& buffer = Read(offset);

  if (!(m_STC->GetFlags() & wxExSTC::STC_WIN_HEX))
  {
    // At least for toggling between hex and non-hex this is necessary to
    // reshow the edge line.
    m_STC->ConfigGet();

    m_STC->SetControlCharSymbol(0);

    const auto message = (get_only_new_data ? SCI_APPENDTEXT: SCI_ADDTEXT);

    // README: The stc.h equivalents AddText, AddTextRaw, InsertText, 
    // InsertTextRaw do not add the length.
    // So for binary files this is the only way for opening.
    m_STC->SendMsg(message, buffer.length(), (wxIntPtr)(const char *)buffer.data());
  }
  else
  {
    m_STC->AddTextHexMode(offset, buffer);
  }

  if (get_only_new_data)
  {
    if (pos_at_end)
    {
      m_STC->DocumentEnd();
    }
  }
  else
  {
    m_STC->GuessType();
    m_STC->DocumentStart();
  }

  if (startPos != endPos)
  {
    // TODO: This does not seem to work.
    m_STC->SetSelection(startPos, endPos);
  }
}

void wxExSTCFile::ResetContentsChanged()
{
  m_STC->SetSavePoint();
}

#endif // wxUSE_GUI
