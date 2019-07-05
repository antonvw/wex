////////////////////////////////////////////////////////////////////////////////
// Name:      ctags.cpp
// Purpose:   Implementation of class wex::ctags
//            https://github.com/universal-ctags/ctags
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <vector>
#include <wx/app.h>
#include <wx/artprov.h>
#include <wx/choicdlg.h>
#include <wx/log.h>
#include <wex/ctags.h>
#include <wex/config.h>
#include <wex/ex.h>
#include <wex/frd.h>
#include <wex/log.h>
#include <wex/managedframe.h>
#include <wex/path.h>
#include <wex/stc.h>
#include <wex/util.h>
#include <readtags.h>

namespace wex
{
  enum image_access_t
  {
    IMAGE_NONE,
    IMAGE_PUBLIC,
    IMAGE_PROTECTED,
    IMAGE_PRIVATE
  };

  // Support class.
  class ctags_info
  {
  public:
    // Constructor.
    ctags_info(const tagEntry& entry)
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
    const std::string name() const {return 
      config(_("vi tag fullpath")).get(false) ?
        m_Path.data().string(): m_Path.fullname();};

    // Opens file in specified frame.
    auto open_file(frame* frame) const
    {
      return frame->open_file(m_Path, 
        control_data().line(m_LineNumber).command(m_Pattern));
    }
  private:
    const path m_Path;
    const int m_LineNumber;
    std::string m_Pattern;
  };

  bool equal(
    const tagEntry& entry, 
    const std::string& text, 
    const std::string& field)
  {
    const char* valuep = tagsField(&entry, field.c_str());

    if (valuep == nullptr)
    {
      return false;
    }
    
    std::string value(valuep);
    
    if (value.find("::") != std::string::npos)
    {
      value = wex::after(value, ':', false);
    }
          
    return text == value;
  }

  const std::string filtered(
    const tagEntry& entry, 
    const wex::ctags_entry& filter)
  {
    if (!filter.is_active()) return entry.name;

    if (!filter.kind().empty())
    { 
      if (entry.kind == nullptr || strcmp(filter.kind().c_str(), entry.kind) != 0)
      {
        return std::string();
      }
    }

    if (!filter.access().empty() && !equal(entry, filter.access(), "access"))
    {
      return std::string();
    }

    if (!filter.class_name().empty() && !equal(entry, filter.class_name(), "class"))
    {
      return std::string();
    }

    if (!filter.signature().empty() && !equal(entry, filter.signature(), "signature"))
    {
      return std::string();
    }

    return entry.name;
  }

  frame* get_frame()
  {
    return dynamic_cast<managed_frame*>(wxTheApp->GetTopWindow());
  }
  
  void set_image(const tagEntry& entry, wex::image_access_t& image)
  {
    if (const char* value = tagsField(&entry, "access"); value != nullptr)
    {
      if (strcmp(value, "public") == 0)
      {
        image = wex::IMAGE_PUBLIC;
      }
      else if (strcmp(value, "protected") == 0)
      {
        image = wex::IMAGE_PROTECTED;
      }
      else if (strcmp(value, "private") == 0)
      {
        image = wex::IMAGE_PRIVATE;
      }
    }
  }

  std::string skip_const(const std::string& text)
  {
    if (text.empty())
      return std::string();
    else if (std::vector<std::string> v; wex::match("(.*) *const$", text, v) == 1)
      return v[0];
    else
      return text;
  }
};

std::map< std::string, wex::ctags_info > wex::ctags::m_Matches;
std::map< std::string, wex::ctags_info >::iterator wex::ctags::m_Iterator;

wex::ctags::ctags(wex::ex* ex, bool open_file)
  : m_Ex(ex)
{
  if (open_file)
  {
    open();
  }
}

const std::string wex::ctags::autocomplete(
  const std::string& text, const ctags_entry& filter)
{
  if (m_File == nullptr) 
  {
    return std::string();
  }

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
    text.c_str(), TAG_PARTIALMATCH | TAG_OBSERVECASE) == TagFailure)
  {
    return std::string();
  }

  if (!m_Prepare)
  {
    autocomplete_prepare();
  }

  std::string s, prev_tag;

  const int min_size{3}, max{100};
  int count {0};
  tagResult result = TagSuccess;

  do
  {
    wex::image_access_t image = IMAGE_NONE;

    if (const auto tag(filtered(entry, filter)); 
      tag.size() > min_size && tag != prev_tag)
    {
      if (!s.empty()) 
      {
        s.append(std::string(1, m_Separator));
      }

      s.append(tag);
      count++;

      set_image(entry, image);

      if (filter.kind() == "f")
      {
        const char* value = tagsField(&entry, "signature");
        
        if (value != nullptr)
        {
          s.append(skip_const(value));
        }
      }

      s.append(image != IMAGE_NONE ? 
        "?" + std::to_string(image): std::string());

      prev_tag = tag;
    } 

    result = (text.empty() ?
      tagsNext(m_File, &entry): tagsFindNext(m_File, &entry));
  } while (result == TagSuccess && count < max);

  log::verbose("ctags autocomplete") << count;

  return s;
}

void wex::ctags::autocomplete_prepare()
{
  m_Ex->get_stc()->AutoCompSetIgnoreCase(false);
  m_Ex->get_stc()->AutoCompSetAutoHide(false);

  wxLogNull logNo;

  m_Ex->get_stc()->RegisterImage(
    IMAGE_PUBLIC, wxArtProvider::GetBitmap(wxART_PLUS));
  m_Ex->get_stc()->RegisterImage(
    IMAGE_PROTECTED, wxArtProvider::GetBitmap(wxART_MINUS));
  m_Ex->get_stc()->RegisterImage(
    IMAGE_PRIVATE, wxArtProvider::GetBitmap(wxART_TICK_MARK));

  m_Prepare = true;
}

void wex::ctags::close()
{
  tagsClose(m_File);
  m_File = nullptr;
}

bool wex::ctags::do_open(const std::string& path)
{
  if (tagFileInfo info; (m_File = tagsOpen(path.c_str(), &info)) != nullptr)
  {
    log::verbose("ctags file") << path;
    return true;
  }

  return false;
}

bool wex::ctags::find(const std::string& tag)
{
  if (m_File == nullptr) return false;

  if (tag.empty())
  {
    log::verbose("ctags find empty tag") << m_Matches.size();
    return next();
  }

  tagEntry entry;
  
  if (tagsFind(m_File, &entry, tag.c_str(), TAG_FULLMATCH) == TagFailure)
  {
    log::status("Tag not found") << tag;
    return false;
  }
  
  m_Matches.clear();

  do
  {
    const ctags_info ct(entry);
    m_Matches.insert({ct.name(), ct});
  } while (tagsFindNext(m_File, &entry) == TagSuccess);

  m_Iterator = m_Matches.begin();

  log::verbose("ctags matches") << m_Matches.size();

  if (m_Matches.size() == 1)
  {
    m_Matches.begin()->second.open_file(get_frame());
  }
  else
  {
    wxArrayString as;
    for (const auto& it : m_Matches) as.Add(it.second.name());
    wxMultiChoiceDialog dialog(get_frame(),
      _("Input") + ":", 
      _("Select File"),
      as);
    if (dialog.ShowModal() != wxID_OK) return false;
    
    for (const auto& sel : dialog.GetSelections())
    {
      m_Iterator = m_Matches.find(as[sel].ToStdString());
      m_Iterator->second.open_file(get_frame());
    }
  }

  find_replace_data::get()->set_find_string(tag);

  return true;
}  

bool master(const tagEntry& entry)
{
  return entry.kind != nullptr && 
     ((strcmp(entry.kind, "c") == 0) ||
      (strcmp(entry.kind, "e") == 0) ||
      (strcmp(entry.kind, "m") == 0));
}

bool wex::ctags::find(const std::string& tag, 
  wex::ctags_entry& current,
  wex::ctags_entry& filter)
{
  if (m_File == nullptr) return false;

  tagEntry entry;

  // Find first entry. This entry determines which kind of
  // filter will be set.
  if (tagsFind(m_File, &entry, tag.c_str(), TAG_FULLMATCH) == TagFailure)
  {
    return false;
  }

  filter.clear();

  do 
  {
    current.kind(entry.kind).class_name(entry.name);

    if (const char* value = tagsField(&entry, "signature"); value != nullptr)
    {
      current.signature(value);
    }

    // If this is not a master entry find next.
    if (master(entry))
    {
      // Set filter for member functions for this member or class.
      if (strcmp(entry.kind, "m") == 0)
      {
        if (const char* value = tagsField(&entry, "typeref"); value != nullptr)
        {
          filter.kind("f").class_name(wex::before(wex::after(value, ':'), ' '));
        }
      }
      else 
      {
        filter.kind("f").class_name(entry.name);
      }

      return true;
    }
  } while (!master(entry) && tagsFindNext(m_File, &entry) == TagSuccess);

  return false;
}

bool wex::ctags::next()
{
  if (m_Matches.size() <= 1)
  {
    log::verbose("ctags no next match") << m_Matches.size();
    return false;
  }

  if (++m_Iterator == m_Matches.end())
  {
    m_Iterator = m_Matches.begin();
  }

  m_Iterator->second.open_file(get_frame());

  return true;
}

void wex::ctags::open(const std::string& filename)
{
  if (m_File != nullptr) return;

  m_Iterator = m_Matches.begin();

  if (filename != DEFAULT_TAGFILE)
  {
    do_open(filename);
  }
  else
  {
    for (const auto & it : std::vector < std::string > {
      "./", config().dir() + "/"})
    {
      if (do_open(it + filename))
      {
        return; // finish, we found a file
      }
    }
  }
  
  if (filename != DEFAULT_TAGFILE && m_File == nullptr)
  {
    log("could not locate ctags file") << filename;
  }
}

bool wex::ctags::previous()
{
  if (m_Matches.size() <= 1)
  {
    log::verbose("ctags no previous match") << m_Matches.size();
    return false;
  }

  if (m_Iterator == m_Matches.begin())
  {
    m_Iterator = m_Matches.end();
  }

  m_Iterator--;
  m_Iterator->second.open_file(get_frame());

  return true;
}
