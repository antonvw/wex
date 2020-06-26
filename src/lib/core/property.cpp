////////////////////////////////////////////////////////////////////////////////
// Name:      property.cpp
// Purpose:   Implementation of wex::property class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/log.h>
#include <wex/property.h>

wex::property::property(const pugi::xml_node& node)
{
  if (!node.empty())
  {
    m_name  = node.attribute("name").value();
    m_value = node.text().get();

    if (m_value.empty())
    {
      log("empty property") << m_name << node;
    }
  }
}

wex::property::property(const std::string& name, const std::string& value)
  : m_name(name)
  , m_value(value)
{
}

void wex::property::apply(wxStyledTextCtrl* stc) const
{
  if (is_ok() && stc->GetParent() != nullptr)
  {
    stc->SetProperty(m_name, m_value);
  }
}

void wex::property::apply_reset(wxStyledTextCtrl* stc) const
{
  if (!m_name.empty() && stc->GetParent() != nullptr)
  {
    stc->SetProperty(m_name, wxEmptyString);
  }
}

bool wex::property::is_ok() const
{
  return !m_name.empty() && !m_value.empty();
}
