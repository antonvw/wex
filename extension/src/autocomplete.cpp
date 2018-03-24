////////////////////////////////////////////////////////////////////////////////
// Name:      autocomplete.cpp
// Purpose:   Implementation of class wxExSTC
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>
#include <wx/extension/autocomplete.h>
#include <wx/extension/ctags.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>
#include <easylogging++.h>

wxExAutoComplete::wxExAutoComplete(wxExSTC* stc)
  : m_STC(stc)
  , m_MinSize(3)
{
}

bool wxExAutoComplete::Activate(const std::string& text)
{
  if (!Use())
  {
    return false;
  }

  bool ret = m_STC->GetVi().GetCTags()->Filter(text, m_Filter);

  if (m_Filter.Active())
  {
    VLOG(9) << "filter active: " << m_Filter.Get();
  }

  if (m_STC->GetVi().GetIsActive())
  {
    const std::string command(text.substr(m_Text.size()));

    if (!command.empty() && !m_STC->GetVi().Command(command))
    {
      LOG(ERROR) << "AutoComplete command failed";
      return false;
    }
  }

  m_Text.clear();

  return ret;
}

bool wxExAutoComplete::Apply(char c)
{
  if (!Use() || m_STC->SelectionIsRectangle())
  {
    return false;
  }

  bool show_ctags = true;
  bool show_inserts = true;
  bool show_keywords = true;
  
  if (c == '.' || 
     (c == '>' && m_STC->GetCharAt(m_STC->GetCurrentPos() - 1) == '-'))
  {
    Clear();
    show_inserts = false;
    show_keywords = false;
  }

  else if (c == WXK_BACK)
  {
    if (m_Text.empty())
    {
      return false;
    }

    m_Text.pop_back();
  }
  else if (isspace(c))
  {
    if (m_Text.size() > m_MinSize)
    {
      m_Inserts.emplace(m_Text);
    }

    Clear();
    return true;
  }
  else if (iscntrl(c) || c == '+')
  {
    return false;
  }
  else
  {
    if (wxExIsCodewordSeparator(
      m_STC->GetCharAt(m_STC->GetCurrentPos() - 1)))
    {
      m_Text = c;
    }
    else
    {
      m_Text += c;
    }
  }

  if (
    !ShowCTags(show_ctags) &&
    !ShowInserts(show_inserts) &&
    !ShowKeywords(show_keywords))
  {
    m_STC->AutoCompCancel();
  }

  return true;
}

void wxExAutoComplete::Clear()
{
  m_Text.clear();
  m_STC->AutoCompCancel();
}

void wxExAutoComplete::Reset()
{
  m_Filter.Clear();
}

bool wxExAutoComplete::ShowCTags(bool show) const
{
  if (!show) return false;

  const std::string comp(m_STC->GetVi().GetCTags()->AutoComplete(
    m_Text, m_Filter));

  if (comp.empty())
  {
    return false;
  }

  m_STC->AutoCompSetSeparator(m_STC->GetVi().GetCTags()->Separator());
  m_STC->AutoCompShow(m_Text.length() - 1, comp);
  m_STC->AutoCompSetSeparator(' ');

  return true;
}

bool wxExAutoComplete::ShowInserts(bool show) const
{
  if (!show) return false;

  if (!m_Text.empty() && !m_Inserts.empty())
  {
    const std::string comp(wxExGetStringSet(
      m_Inserts, m_MinSize, m_Text));
      
    if (!comp.empty())
    {
      m_STC->AutoCompShow(m_Text.length() - 1, comp);
      return true;
    }
  }

  return false;
}

bool wxExAutoComplete::ShowKeywords(bool show) const
{
  if (!show) return false;

  if (!m_Text.empty() && m_STC->GetLexer().KeywordStartsWith(m_Text))
  {
    const std::string comp(
      m_STC->GetLexer().GetKeywordsString(-1, m_MinSize, m_Text));
      
    if (!comp.empty())
    {
      m_STC->AutoCompShow(m_Text.length() - 1, comp);
      return true;
    }
  }

  return false;
}

bool wxExAutoComplete::Use() const
{
  return m_Use && wxConfigBase::Get()->ReadBool(_("Autocomplete"), true);
}
