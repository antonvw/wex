////////////////////////////////////////////////////////////////////////////////
// Name:      ctags.cpp
// Purpose:   Implementation of class wxExCTags
//            https://github.com/universal-ctags/ctags
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <vector>
#include <wx/artprov.h>
#include <wx/choicdlg.h>
#include <wx/config.h>
#include <wx/log.h>
#include <wx/extension/ctags.h>
#include <wx/extension/ex.h>
#include <wx/extension/frd.h>
#include <wx/extension/log.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/path.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>
#include <easylogging++.h>
#include "readtags.h"

enum wxExImageAccessType
{
  IMAGE_NONE,
  IMAGE_PUBLIC,
  IMAGE_PROTECTED,
  IMAGE_PRIVATE
};

// Support class.
class wxExCTagsEntry
{
public:
  // Constructor.
  wxExCTagsEntry(const tagEntry& entry)
    : m_LineNumber(entry.address.lineNumber)
    , m_Path(entry.file)
    , m_Pattern(entry.address.pattern != nullptr ? 
      // prepend colon to force ex command
      ":" + std::string(entry.address.pattern): std::string()) {
    // replace any * with ., somehow the pattern generated by
    // ctags mixes regex with non regex....
    std::replace(m_Pattern.begin(), m_Pattern.end(), '*', '.');};

  // Returns name, being fullpath or path name depending on
  // config settings.
  const std::string GetName() const {return 
    wxConfigBase::Get()->ReadBool(_("vi tag fullpath"), false) ?
      m_Path.Path().string(): m_Path.GetFullName();};

  // Opens file in specified frame.
  auto OpenFile(wxExFrame* frame) const
  {
    return frame->OpenFile(m_Path, 
      wxExControlData().Line(m_LineNumber).Command(m_Pattern));
  }
private:
  const wxExPath m_Path;
  const int m_LineNumber;
  std::string m_Pattern;
};

void SetImage(const tagEntry& entry, wxExImageAccessType& image)
{
  const char* value = tagsField(&entry, "access");
  if (value == nullptr) return;

  if (strcmp(value, "public") == 0)
  {
    image = IMAGE_PUBLIC;
  }
  else if (strcmp(value, "protected") == 0)
  {
    image = IMAGE_PROTECTED;
  }
  else if (strcmp(value, "private") == 0)
  {
    image = IMAGE_PRIVATE;
  }
}

bool Compare(const tagEntry& entry, 
  const std::string& text, const std::string& field)
{
  const char* value = tagsField(&entry, field.c_str());
  return 
    value != nullptr && strcmp(text.c_str(), value) == 0;
}

const std::string Filtered(
  const tagEntry& entry, 
  const wxExCTagsFilter& filter, 
  wxExImageAccessType& image)
{
  if (!filter.Active()) return entry.name;

  if (!filter.Kind().empty())
  { 
    if (entry.kind == nullptr || strcmp(filter.Kind().c_str(), entry.kind) != 0)
    {
      return std::string();
    }
  }

  if (!filter.Access().empty() && !Compare(entry, filter.Access(), "access"))
  {
    return std::string();
  }

  if (!filter.Class().empty() && !Compare(entry, filter.Class(), "class"))
  {
    return std::string();
  }

  if (!filter.Signature().empty() && !Compare(entry, filter.Signature(), "signature"))
  {
    return std::string();
  }

  return entry.name;
}

wxExCTags::wxExCTags(wxExEx* ex)
  : m_Ex(ex)
  , m_Frame(ex->GetFrame())
  , m_Iterator(m_Matches.begin())
  , m_Separator(3)
{
  Init(m_Ex->GetCommand().STC()->GetData().CTagsFileName());
}

wxExCTags::wxExCTags(wxExFrame* frame)
  : m_Frame(frame)
  , m_Iterator(m_Matches.begin())
  , m_Separator(3)
{
  Init(DEFAULT_TAGFILE);
}

wxExCTags::~wxExCTags()
{
  tagsClose(m_File);
}

std::string wxExCTags::AutoComplete(
  const std::string& text, const wxExCTagsFilter& filter)
{
  if (m_File == nullptr) return std::string();

  tagEntry entry;
  
  if (text.empty())
  { 
    if (tagsFirst(m_File, &entry) == TagFailure)
    {
      return std::string();
    }
  }
  else if (tagsFind(
    m_File, 
    &entry, 
    text.c_str(), TAG_PARTIALMATCH | TAG_IGNORECASE) == TagFailure)
  {
    return std::string();
  }

  std::string s, prev_tag;

  const int max{100};
  int count {0};
  tagResult result = TagSuccess;

  if (!m_Prepare)
  {
    AutoCompletePrepare();
  }

  do
  {
    wxExImageAccessType image = IMAGE_NONE;
    const std::string tag(Filtered(entry, filter, image));

    if (!tag.empty() && tag != prev_tag)
    {
      if (!s.empty()) s.append(std::string(1, m_Separator));

      s.append(tag);
      count++;

      SetImage(entry, image);

      if (filter.Kind() == "f")
      {
        s.append(tagsField(&entry, "signature"));
      }

      s.append(image != IMAGE_NONE ? "?" + std::to_string(image): std::string());

      prev_tag = tag;
    } 

    result = (text.empty() ?
      tagsNext(m_File, &entry): tagsFindNext(m_File, &entry));
  } while (result == TagSuccess && count < max);

  VLOG(9) << "ctags AutoComplete: " << count;

  return s;
}

void wxExCTags::AutoCompletePrepare()
{
  m_Ex->GetSTC()->AutoCompSetIgnoreCase(true);
  m_Ex->GetSTC()->AutoCompSetAutoHide(false);

  wxLogNull logNo;
  m_Ex->GetSTC()->RegisterImage(IMAGE_PUBLIC, wxArtProvider::GetBitmap(wxART_PLUS));
  m_Ex->GetSTC()->RegisterImage(IMAGE_PROTECTED, wxArtProvider::GetBitmap(wxART_MINUS));
  m_Ex->GetSTC()->RegisterImage(IMAGE_PRIVATE, wxArtProvider::GetBitmap(wxART_TICK_MARK));

  m_Prepare = true;
}

bool Master(const tagEntry& entry)
{
  return entry.kind != nullptr && 
     ((strcmp(entry.kind, "c") == 0) ||
      (strcmp(entry.kind, "e") == 0) ||
      (strcmp(entry.kind, "m") == 0));
}

bool wxExCTags::Filter(const std::string& name, wxExCTagsFilter& filter) const
{
  if (m_File == nullptr) return false;

  tagEntry entry;

  // Find first entry. This entry determines which kind of
  // filter will be set.
  if (tagsFind(m_File, &entry, name.c_str(), TAG_FULLMATCH) == TagFailure)
  {
    return false;
  }

  // If this is not a master entry find next.
  while (!Master(entry) && tagsFindNext(m_File, &entry) == TagSuccess)
  {
  }

  if (!Master(entry))
  {
    filter.Clear();
    return false;
  }

  // Set filter for member functions for this member or class.
  if (strcmp(entry.kind, "m") == 0)
  {
    const char* value = tagsField(&entry, "typeref");

    if (value != nullptr)
    {
      filter.Kind("f").Class(wxExBefore(wxExAfter(value, ':'), ' '));
    }
  }
  else 
  {
    filter.Kind("f").Class(entry.name);
  }

  return true;
}

bool wxExCTags::Find(const std::string& name)
{
  if (m_File == nullptr) return false;

  if (name.empty())
  {
    return Next();
  }

  tagEntry entry;
  
  if (tagsFind(m_File, &entry, name.c_str(), TAG_FULLMATCH) == TagFailure)
  {
    wxLogStatus("tag not found: " + wxString(name));
    return false;
  }
  
  m_Matches.clear();

  do
  {
    const wxExCTagsEntry ct(entry);
    m_Matches.insert({ct.GetName(), ct});
  } while (tagsFindNext(m_File, &entry) == TagSuccess);

  m_Iterator = m_Matches.begin();

  VLOG(9) << "ctags matches: " << m_Matches.size();

  if (m_Matches.size() == 1)
  {
    m_Matches.begin()->second.OpenFile(m_Frame);
  }
  else
  {
    wxArrayString as;
    for (const auto& it : m_Matches) as.Add(it.second.GetName());
    wxMultiChoiceDialog dialog(m_Frame,
      _("Input") + ":", 
      _("Select File"),
      as);
    if (dialog.ShowModal() != wxID_OK) return false;
    
    for (const auto& sel : dialog.GetSelections())
    {
      m_Iterator = m_Matches.find(as[sel].ToStdString());
      m_Iterator->second.OpenFile(m_Frame);
    }
  }

  wxExFindReplaceData::Get()->SetFindString(name);

  return true;
}  

void wxExCTags::Init(const std::string& filename)
{
  wxExPath path(filename);

  if (path.IsAbsolute())
  {
    // an absolute file should exist
    Open(path.Path().string(), true);
  }
  else
  {
    // First check whether default tagfile with extension 
    // exists, then without extension.
    for (const auto & it : std::vector < std::string > {
      "./", wxExConfigDir() + "/"})
    {
      if (
        (m_Ex != nullptr && (
           Open(it + filename + m_Ex->GetSTC()->GetFileName().GetExtension()))) ||
        Open(it + filename))
      {
        return; // finish, we found a file
      }
    }

    if (filename != DEFAULT_TAGFILE && m_File == nullptr)
    {
      wxExLog() << "could not open ctags file:" << filename;
    }
  }
}

bool wxExCTags::Next()
{
  if (m_Matches.size() <= 1)
  {
    return false;
  }

  if (++m_Iterator == m_Matches.end())
  {
    m_Iterator = m_Matches.begin();
  }

  m_Iterator->second.OpenFile(m_Frame);

  return true;
}

bool wxExCTags::Open(const std::string& path, bool show_error)
{
  tagFileInfo info;

  if ((m_File = tagsOpen(path.c_str(), &info)) != nullptr)
  {
    VLOG(9) << "ctags file: " << path;
    return true;
  }
  else if (show_error)
  {
    wxExLog() << "could not open ctags file:" << path;
  }

  return false;
}

bool wxExCTags::Previous()
{
  if (m_Matches.size() <= 1)
  {
    return false;
  }

  if (m_Iterator == m_Matches.begin())
  {
    m_Iterator = m_Matches.end();
  }

  m_Iterator--;
  m_Iterator->second.OpenFile(m_Frame);

  return true;
}
