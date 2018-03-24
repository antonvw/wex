////////////////////////////////////////////////////////////////////////////////
// Name:      ctags-filter.cpp
// Purpose:   Implementation of class wxExCTagsFilter
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/ctags-filter.h>
#include <easylogging++.h>

wxExCTagsFilter& wxExCTagsFilter::Access(const std::string& v) 
{
  m_access = v;
  return *this;
}

bool wxExCTagsFilter::Active() const
{
  return 
    !m_access.empty() || !m_class.empty() ||
    !m_kind.empty() || !m_signature.empty();
}

wxExCTagsFilter& wxExCTagsFilter::Class(const std::string& v)
{
  m_class = v;
  return *this;
}

void wxExCTagsFilter::Clear()
{
  VLOG(9) << "filter: " << Get() << " cleared";

  m_access.clear();
  m_class.clear();
  m_kind.clear();
  m_signature.clear();
}

const std::string wxExCTagsFilter::Get() const
{
  return 
    (!m_access.empty() ? "access: " + m_access + " ": std::string()) + 
    (!m_class.empty() ? "class: " + m_class + " ": std::string()) +
    (!m_kind.empty() ? "kind: " + m_kind + " ": std::string()) +
    (!m_signature.empty() ? "signature: " + m_signature + " ": std::string());
}

wxExCTagsFilter& wxExCTagsFilter::Kind(const std::string& v)
{
  m_kind = v;
  return *this;
}

wxExCTagsFilter& wxExCTagsFilter::Signature(const std::string& v)
{
  m_signature = v;
  return *this;
}
