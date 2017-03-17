////////////////////////////////////////////////////////////////////////////////
// Name:      ctags.cpp
// Purpose:   Implementation of class wxExCTags
//            http://ctags.sourceforge.net/ctags.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <map>
#include <vector>
#include <wx/choicdlg.h>
#include <wx/log.h>
#include <wx/extension/ctags.h>
#include <wx/extension/filename.h>
#include <wx/extension/frame.h>
#include <wx/extension/util.h>
#include "readtags.h"

class wxExCTagsEntry
{
public:
  wxExCTagsEntry(const tagEntry& entry)
    : m_LineNumber(entry.address.lineNumber)
    , m_FileName(entry.file)
    , m_Pattern(entry.address.pattern != nullptr ? 
      // prepend colon to force ex command
      ":" + std::string(entry.address.pattern): std::string()) {
    // replace any * with ., somehow the pattern generated by
    // ctags mixes regex with non regex....
    std::replace(m_Pattern.begin(), m_Pattern.end(), '*', '.');};
  const std::string GetName() const {return m_FileName.GetFullName();};
  void OpenFile(wxExFrame* frame) const
  {
    frame->OpenFile(m_FileName, 
      m_LineNumber, std::string(), 0, STC_WIN_DEFAULT, m_Pattern);
  }
private:
  const wxExFileName m_FileName;
  const int m_LineNumber;
  std::string m_Pattern;
};

wxExCTags::wxExCTags(wxExFrame* frame, const std::string& filename)
  : m_Frame(frame)
{
  tagFileInfo info;

  for (const auto & it : std::vector < std::string > {
    "./", wxExConfigDir() + "/"})
  {
    if ((m_File = tagsOpen(std::string(it + filename).c_str(), &info)) != nullptr)
    {
      return; // finish, we found a file
    }
  }
}

wxExCTags::~wxExCTags()
{
  tagsClose(m_File);
}

bool wxExCTags::Find(const std::string& name) const
{
  if (m_File == nullptr || name.empty()) return false;

  tagEntry entry;
  
  if (tagsFind(m_File, &entry, name.c_str(), TAG_FULLMATCH) == TagFailure)
  {
    wxLogStatus("tag not found: " + wxString(name));
    return false;
  }
  
  const wxExCTagsEntry ct(entry);
  
  std::map< std::string, wxExCTagsEntry > v{{ct.GetName(), ct}};
  
  while (tagsFindNext(m_File, &entry) == TagSuccess)
  {
    const wxExCTagsEntry ct(entry);
    v.insert({ct.GetName(), ct});
  }

  if (v.size() == 1)
  {
    v.begin()->second.OpenFile(m_Frame);
  }
  else
  {
    wxArrayString as;
    for (const auto& it : v) as.Add(it.second.GetName());
    wxSingleChoiceDialog dialog(m_Frame,
      _("Input") + ":", 
      _("Select File"),
      as);
    if (dialog.ShowModal() != wxID_OK) return false;
    
    const auto & it = v.find(dialog.GetStringSelection().ToStdString());
    it->second.OpenFile(m_Frame);
  }

  return true;
}  
