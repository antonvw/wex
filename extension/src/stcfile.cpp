////////////////////////////////////////////////////////////////////////////////
// Name:      stc_file.cpp
// Purpose:   Implementation of class wex::stc_file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <pugixml.hpp>
#include <wx/extension/stcfile.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/lexers.h>
#include <wx/extension/log.h>
#include <wx/extension/path.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h> // for STAT_ etc.
#include <easylogging++.h>

void CheckWellFormed(wex::stc* stc, const wex::path& fn)
{
  if (fn.GetLexer().GetLanguage() == "xml")
  {
    if (const pugi::xml_parse_result result = 
      pugi::xml_document().load_file(fn.Path().string().c_str());
      !result)
    {
      wex::xml_error(fn, &result, stc);
    }
  }
}

wex::stc_file::stc_file(stc* stc, const std::string& filename)
  : file(filename)
  , m_STC(stc)
{
}

bool wex::stc_file::DoFileLoad(bool synced)
{
  if (
   GetContentsChanged() &&
   file_dialog(this).ShowModalIfChanged() == wxID_CANCEL)
  {
    return false;
  }

  // Synchronizing by appending only new data only works for log files.
  // Other kind of files might get new data anywhere inside the file,
  // we cannot sync that by keeping pos. 
  // Also only do it for reasonably large files.
  const bool isLog = (GetFileName().GetExtension().find(".log") == 0);
  
  m_STC->UseModificationMarkers(false);

  ReadFromFile(
    synced &&
    isLog &&
    m_STC->GetTextLength() > 1024);

  if (!synced)
  {
    // ReadFromFile might already have set the lexer using a modeline.
    if (m_STC->GetLexer().GetScintillaLexer().empty())
    {
      m_STC->GetLexer().Set(GetFileName().GetLexer(), true);
    }

    wxLogStatus(_("Opened") + ": " + GetFileName().Path().string());
    VLOG(1) << "opened: " << GetFileName().Path().string();
  }
  
  m_STC->PropertiesMessage(synced ? STAT_SYNC: STAT_DEFAULT);
  m_STC->UseModificationMarkers(true);
  
  CheckWellFormed(m_STC, GetFileName());
  
  return true;
}

void wex::stc_file::DoFileNew()
{
  m_STC->SetName(GetFileName().Path().string());
  m_STC->PropertiesMessage();
  m_STC->ClearDocument();
  m_STC->GetLexer().Set(GetFileName().GetLexer(), true); // allow fold
}

void wex::stc_file::DoFileSave(bool save_as)
{
  if (m_STC->GetHexMode().Active())
  {
    Write(m_STC->GetHexMode().GetBuffer());
  }
  else
  {
    Write(m_STC->GetTextRaw().data(), m_STC->GetTextRaw().length());
  }
  
  if (save_as)
  {
    m_STC->SetReadOnly(GetFileName().IsReadOnly());
    m_STC->GetLexer().Set(GetFileName().GetLexer());
    m_STC->SetName(GetFileName().Path().string());
  }
  
  m_STC->MarkerDeleteAllChange();
  
  wxLogStatus(_("Saved") + ": " + GetFileName().Path().string());
  VLOG(1) << "saved: " << GetFileName().Path().string();
  
  CheckWellFormed(m_STC, GetFileName());
}

bool wex::stc_file::GetContentsChanged() const 
{
  return m_STC->GetModify();
}

void wex::stc_file::ReadFromFile(bool get_only_new_data)
{
  const bool pos_at_end = (m_STC->GetCurrentPos() >= m_STC->GetTextLength() - 1);

  int startPos, endPos;
  m_STC->GetSelection(&startPos, &endPos);

  std::streampos offset = 0;

  if (m_PreviousLength < m_STC->GetFileName().GetStat().st_size && get_only_new_data)
  {
    offset = m_PreviousLength;
  }

  if (offset == std::streampos(0))
  {
    m_STC->ClearDocument();
  }

  m_PreviousLength = m_STC->GetFileName().GetStat().st_size;

  if (const auto buffer = Read(offset); buffer != nullptr)
  {
    if (!m_STC->GetHexMode().Active())
    {
      m_STC->Allocate(buffer->length());
    
      get_only_new_data ? 
        m_STC->AppendTextRaw((const char *)buffer->data(), buffer->length()):
        m_STC->AddTextRaw((const char *)buffer->data(), buffer->length());
    }
    else
    {
      m_STC->GetHexMode().AppendText(std::string(buffer->data(), buffer->length()));
    }
  }
  else
  {
     m_STC->SetText("READ FAILED");
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
    m_STC->SetSelection(startPos, endPos);
  }
  
  m_STC->EmptyUndoBuffer();
}

void wex::stc_file::ResetContentsChanged()
{
  m_STC->SetSavePoint();
}
