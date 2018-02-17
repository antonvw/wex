////////////////////////////////////////////////////////////////////////////////
// Name:      ctags.cpp
// Purpose:   Implementation of class wxExCTags
//            https://github.com/universal-ctags/ctags
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <vector>
#include <easylogging++.h>
#include <wx/artprov.h>
#include <wx/choicdlg.h>
#include <wx/config.h>
#include <wx/log.h>
#include <wx/extension/ctags.h>
#include <wx/extension/frame.h>
#include <wx/extension/frd.h>
#include <wx/extension/path.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>
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

void SetImage(wxExImageAccessType& image, const char* value)
{
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

const std::string Filtered(
  const tagEntry& entry, 
  const wxExCTagsFilter& filter, 
  wxExImageAccessType& image)
{
  if (!filter.Active()) return entry.name;

  if (filter.Kind().empty() && entry.fields.count == 0)
  {
    LOG(ERROR) << "filter active, but no fields present";
  }

  if (!filter.Kind().empty())
  { 
    if (entry.kind == nullptr || strcmp(filter.Kind().c_str(), entry.kind) != 0)
    {
      return std::string();
    }
  }

  bool found = true;

  if (!filter.Access().empty() || !filter.Class().empty() || !filter.Signature().empty())
  {
    found = false;

    for (int i = 0; i < entry.fields.count; ++i)
    {
      const char* key = entry.fields.list[i].key;
      const char* value = entry.fields.list[i].value;

      if (strcmp(key, "access") == 0)
      {
        SetImage(image, value);

        if (!filter.Access().empty())
        {
          if (filter.Access() != value)
          {
            return std::string();
          }
          else
          {
            found = true;
          }
        }
      }
      else if (strcmp(key, "class") == 0)
      {
        if (!filter.Class().empty())
        {
          if (filter.Class() != value)
          {
            return std::string();
          }
          else
          {
            found = true;
          }
        }
      }
      else if (strcmp(key, "signature") == 0)
      {
        if (!filter.Signature().empty())
        {
          if (filter.Signature() != value)
          {
            return std::string();
          }
          else
          {
            found = true;
          }
        }
      }
    }
  }

  if (!found)
  {
    return std::string();
  }

  return entry.name;
}

wxExCTags::wxExCTags(wxExFrame* frame, const std::string& filename)
  : m_Frame(frame)
  , m_Iterator(m_Matches.begin())
  , m_Separator(3)
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

std::string wxExCTags::AutoComplete(
  const std::string& text, const wxExCTagsFilter& filter) const
{
  if (m_File == nullptr) return std::string();

  tagEntry entry;
  
  if (tagsFind(m_File, &entry, text.c_str(), TAG_PARTIALMATCH) == TagFailure)
  {
    return std::string();
  }

  std::string s, prev_tag;

  do
  {
    wxExImageAccessType image = IMAGE_NONE;
    const std::string tag(Filtered(entry, filter, image));

    if (!tag.empty() && tag != prev_tag)
    {
      if (!s.empty()) s.append(std::string(1, m_Separator));

      s.append(tag);

      if (filter.Kind() == "f")
      {
        for (int i = 0; i < entry.fields.count; ++i)
        {
          if (strcmp(entry.fields.list[i].key, "signature") == 0)
          {
            s.append(entry.fields.list[i].value);
          }
        }
      }

      s.append(image != IMAGE_NONE ? "?" + std::to_string(image): std::string());

      prev_tag = tag;
    } 
  } while (tagsFindNext(m_File, &entry) == TagSuccess);

  return s;
}

void wxExCTags::AutoCompletePrepare(wxExSTC* stc)
{
  if (stc == nullptr)
  {
    LOG(ERROR) << "missing STC";
    return;
  }

  stc->AutoCompSetIgnoreCase(true);
  stc->AutoCompSetAutoHide(false);

  wxLogNull logNo;
  stc->RegisterImage(IMAGE_PUBLIC, wxArtProvider::GetBitmap(wxART_PLUS));
  stc->RegisterImage(IMAGE_PROTECTED, wxArtProvider::GetBitmap(wxART_MINUS));
  stc->RegisterImage(IMAGE_PRIVATE, wxArtProvider::GetBitmap(wxART_TICK_MARK));
}

bool Master(const tagEntry& entry)
{
  return entry.kind != nullptr && 
     ((strcmp(entry.kind, "c") == 0) ||
      (strcmp(entry.kind, "e") == 0));
}

bool wxExCTags::Filter(const std::string& name, wxExCTagsFilter& filter) const
{
  tagEntry entry;

  // Find first entry.  
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

  // Set filter for member functions for this class.
  filter.Kind("f").Class(entry.name);

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
