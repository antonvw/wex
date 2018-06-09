////////////////////////////////////////////////////////////////////////////////
// Name:      ctags-entry.cpp
// Purpose:   Implementation of class wxExCTagsEntry
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/ctags-entry.h>
#include <wx/extension/log.h>
#include <easylogging++.h>

wxExCTagsEntry& wxExCTagsEntry::Access(const std::string& v) 
{
  m_access = v;
  return *this;
}

bool wxExCTagsEntry::Active() const
{
  return 
    !m_access.empty() || !m_class.empty() ||
    !m_kind.empty() || !m_signature.empty();
}

wxExCTagsEntry& wxExCTagsEntry::Class(const std::string& v)
{
  m_class = v;
  return *this;
}

void wxExCTagsEntry::Clear()
{
  VLOG(9) << "filter: " << Get() << " cleared";

  m_access.clear();
  m_class.clear();
  m_kind.clear();
  m_signature.clear();
}

const std::string wxExCTagsEntry::Get() const
{
  return 
    (!m_access.empty() ? "access: " + m_access + " ": std::string()) + 
    (!m_class.empty() ? "class: " + m_class + " ": std::string()) +
    (!m_kind.empty() ? "kind: " + m_kind + " ": std::string()) +
    (!m_signature.empty() ? "signature: " + m_signature + " ": std::string());
}

wxExCTagsEntry& wxExCTagsEntry::Kind(const std::string& v)
{
  m_kind = v;
  return *this;
}

wxExCTagsEntry& wxExCTagsEntry::Signature(const std::string& v)
{
  m_signature = v;
  return *this;
}
