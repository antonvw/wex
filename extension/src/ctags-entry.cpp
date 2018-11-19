////////////////////////////////////////////////////////////////////////////////
// Name:      ctags-entry.cpp
// Purpose:   Implementation of class wex::ctags_entry
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ctags-entry.h>
#include <wex/log.h>
#include <easylogging++.h>

wex::ctags_entry& wex::ctags_entry::access(const std::string& v) 
{
  m_access = v;
  return *this;
}

wex::ctags_entry& wex::ctags_entry::class_name(const std::string& v)
{
  m_class = v;
  return *this;
}

void wex::ctags_entry::clear()
{
  VLOG(9) << "filter: " << get() << " cleared";

  m_access.clear();
  m_class.clear();
  m_kind.clear();
  m_signature.clear();
}

const std::string wex::ctags_entry::get() const
{
  return 
    (!m_access.empty() ? "access: " + m_access + " ": std::string()) + 
    (!m_class.empty() ? "class: " + m_class + " ": std::string()) +
    (!m_kind.empty() ? "kind: " + m_kind + " ": std::string()) +
    (!m_signature.empty() ? "signature: " + m_signature + " ": std::string());
}

bool wex::ctags_entry::is_active() const
{
  return 
    !m_access.empty() || !m_class.empty() ||
    !m_kind.empty() || !m_signature.empty();
}

wex::ctags_entry& wex::ctags_entry::kind(const std::string& v)
{
  m_kind = v;
  return *this;
}

wex::ctags_entry& wex::ctags_entry::signature(const std::string& v)
{
  m_signature = v;
  return *this;
}
