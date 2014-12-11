////////////////////////////////////////////////////////////////////////////////
// Name:      stcfile.cpp
// Purpose:   Implementation of class wxExSTCFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/xml/xml.h>
#include <wx/extension/stcfile.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/filename.h>
#include <wx/extension/lexers.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h> // for STAT_ etc.

#if wxUSE_GUI
void CheckWellFormed(const wxExFileName& fn)
{
  if (
    wxExLexers::Get()->GetFileName() != fn &&
   (fn.GetLexer().GetDisplayLexer() == "xml" ||
    fn.GetLexer().GetDisplayLexer() == "xsd" ||
    fn.GetLexer().GetDisplayLexer() == "xsl"))
  {
    if (!wxXmlDocument(fn.GetFullPath()).IsOk())
    {
      wxLogStatus("not a valid XML document");
    }
  }
}

wxExSTCFile::wxExSTCFile(wxExSTC* stc, const wxString& filename)
  : m_STC(stc)
  , m_PreviousLength(0)
{
  if (!filename.empty())
  {
    Assign(wxExFileName(filename));
  }
}

bool wxExSTCFile::DoFileLoad(bool synced)
{
  if (
   GetContentsChanged() &&
   wxExFileDialog(m_STC, this).ShowModalIfChanged() == wxID_CANCEL)
  {
    return false;
  }

  // Synchronizing by appending only new data only works for log files.
  // Other kind of files might get new data anywhere inside the file,
  // we cannot sync that by keeping pos. 
  // Also only do it for reasonably large files.
  const bool isLog = (GetFileName().GetExt().CmpNoCase("log") == 0);
  
  m_STC->UseModificationMarkers(false);

  ReadFromFile(
    synced &&
    isLog &&
    m_STC->GetTextLength() > 1024);

  if (!synced)
  {
    m_STC->SetLexer(GetFileName().GetLexer(), true);

    // No edges for log files.
    if (isLog)
    {
      m_STC->SetEdgeMode(wxSTC_EDGE_NONE);
    }
    
    wxLogStatus(_("Opened") + ": " + GetFileName().GetFullPath());
  }
  
  CheckWellFormed(GetFileName());
  
  m_STC->PropertiesMessage(synced ? STAT_SYNC: STAT_DEFAULT);
  m_STC->UseModificationMarkers(true);
  
  return true;
}

void wxExSTCFile::DoFileNew()
{
  m_STC->SetName(GetFileName().GetFullPath());
  m_STC->PropertiesMessage();
  m_STC->ClearDocument();
  m_STC->SetLexer(GetFileName().GetLexer(), true); // allow fold
}

void wxExSTCFile::DoFileSave(bool save_as)
{
  size_t size;
  size_t count;
  
  if (m_STC->GetHexMode().Active())
  {
    count = m_STC->GetHexMode().GetBuffer().size();
    size = Write(m_STC->GetHexMode().GetBuffer(), count);
  }
  else
  {
    const wxCharBuffer& buffer = m_STC->GetTextRaw(); 
    count = buffer.length();
    size = Write(buffer.data(), count);
  }
  
  if (size != count)
  {
    wxLogStatus("could not save file");
    return;
  }
  
  if (save_as)
  {
    m_STC->SetReadOnly(!GetFileName().IsFileWritable());
    m_STC->SetLexer(GetFileName().GetLexer());
    m_STC->SetName(GetFileName().GetFullPath());
  }
  
  m_STC->MarkerDeleteAllChange();
  
  wxLogStatus(_("Saved") + ": " + GetFileName().GetFullPath());
  
  CheckWellFormed(GetFileName());
}

bool wxExSTCFile::GetContentsChanged() const 
{
  return m_STC->GetModify();
}

bool wxExSTCFile::Read(const wxString& name) const
{
  wxFileName fn(name);

  if (fn.IsRelative())
  {
    fn.Normalize(wxPATH_NORM_ALL, GetFileName().GetPath());
  }
  
  wxExFile file;

  if (file.Exists(fn.GetFullPath()))
  {
    if (file.Open(fn.GetFullPath()))
    {
      const wxCharBuffer& buffer = file.Read();
      m_STC->AddTextRaw((const char *)buffer.data(), buffer.length());
      
      return true;
    }
  }
  
  wxLogStatus(_("file: %s does not exist"), name);
  
  return false;
}

void wxExSTCFile::ReadFromFile(bool get_only_new_data)
{
  // Be sure we can add text.
  m_STC->SetReadOnly(false);

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

  const wxCharBuffer& buffer = wxExFile::Read(offset);

  if (!m_STC->GetHexMode().Active())
  {
    m_STC->Allocate(buffer.length());
    
    get_only_new_data ? 
      m_STC->AppendTextRaw((const char *)buffer.data(), buffer.length()):
      m_STC->AddTextRaw((const char *)buffer.data(), buffer.length());
  }
  else
  {
    m_STC->GetHexMode().AppendText(buffer);
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
    
#ifdef DEBUG
    std::cout << GetFileName().GetFullPath() << "\n";
#endif
    m_STC->DocumentStart();
  }

  if (startPos != endPos)
  {
    m_STC->SetSelection(startPos, endPos);
  }
  
  if (( m_STC->GetFlags() & m_STC->STC_WIN_READ_ONLY) ||
       !GetFileName().IsFileWritable())
  {
    m_STC->SetReadOnly(true);
  }

  m_STC->EmptyUndoBuffer();
}

void wxExSTCFile::ResetContentsChanged()
{
  m_STC->SetSavePoint();
}

#endif // wxUSE_GUI
