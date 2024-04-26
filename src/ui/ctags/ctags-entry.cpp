////////////////////////////////////////////////////////////////////////////////
// Name:      ctags-entry.cpp
// Purpose:   Implementation of class wex::ctags_entry
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/core/regex.h>
#include <wex/ctags/ctags-entry.h>
#include <wx/artprov.h>
#include <wx/stc/stc.h>

namespace wex
{
enum class image_access_t
{
  NONE,
  PUBLIC,
  PROTECTED,
  PRIVATE
};
}

wex::ctags_entry::ctags_entry()
  : m_reflect(
      {REFLECT_ADD("access", m_access),
       REFLECT_ADD("class", m_class),
       REFLECT_ADD("kind", m_kind),
       REFLECT_ADD("signature", m_signature)},
      reflection::log_t::SKIP_EMPTY)
{
}

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
  memset(&m_entry, 0, sizeof(m_entry));

  if (!is_active())
  {
    return;
  }

  m_access.clear();
  m_class.clear();
  m_kind.clear();
  m_signature.clear();

  log::trace("filter") << log() << " cleared";
}

bool wex::ctags_entry::entry_equal(
  const std::string& text,
  const std::string& field) const
{
  if (const auto* valuep = tagsField(&m_entry, field.c_str());
      valuep != nullptr)
  {
    if (std::string value(valuep); value.contains("::"))
    {
      value = wex::rfind_after(value, ":");
      return text == value;
    }
  }

  return false;
}

const std::string wex::ctags_entry::entry_string(size_t min_size) const
{
  std::string s;

  if (const auto& tag(filter()); tag.size() > min_size)
  {
    s.append(tag + signature_and_image());
  }

  return s;
}

const std::string wex::ctags_entry::filter() const
{
  if (!is_active())
  {
    return m_entry.name == nullptr ? std::string() : m_entry.name;
  }

  if (!kind().empty())
  {
    if (m_entry.kind == nullptr || strcmp(kind().c_str(), m_entry.kind) != 0)
    {
      return std::string();
    }
  }

  if (!access().empty() && !entry_equal(access(), "access"))
  {
    return std::string();
  }

  if (!class_name().empty() && !entry_equal(class_name(), "class"))
  {
    return std::string();
  }

  if (!signature().empty() && !entry_equal(signature(), "signature"))
  {
    return std::string();
  }

  return m_entry.name;
}

wex::ctags_entry& wex::ctags_entry::filter(const ctags_entry& entry)
{
  // Set filter for member functions for this member or class.
  if (strcmp(entry.m_entry.kind, "m") == 0)
  {
    if (const auto* value = tagsField(&entry.m_entry, "typeref");
        value != nullptr)
    {
      kind("f").class_name(wex::find_before(wex::find_after(value, ":"), " "));
    }
  }
  else
  {
    kind("f").class_name(entry.m_entry.name);
  }

  return *this;
}

std::string wex::ctags_entry::image_string() const
{
  image_access_t image = image_access_t::NONE;

  if (const auto* value = tagsField(&m_entry, "access"); value != nullptr)
  {
    if (strcmp(value, "public") == 0)
    {
      image = image_access_t::PUBLIC;
    }
    else if (strcmp(value, "protected") == 0)
    {
      image = image_access_t::PROTECTED;
    }
    else if (strcmp(value, "private") == 0)
    {
      image = image_access_t::PRIVATE;
    }
  }

  return image != image_access_t::NONE ?
           "?" + std::to_string(std::to_underlying(image)) :
           std::string();
}

bool wex::ctags_entry::is_active() const
{
  return !m_access.empty() || !m_class.empty() || !m_kind.empty() ||
         !m_signature.empty();
}

bool wex::ctags_entry::is_master() const
{
  return m_entry.kind != nullptr &&
         ((strcmp(m_entry.kind, "c") == 0) ||
          (strcmp(m_entry.kind, "e") == 0) || (strcmp(m_entry.kind, "m") == 0));
}

wex::ctags_entry& wex::ctags_entry::kind(const std::string& v)
{
  m_kind = v;
  return *this;
}

void wex::ctags_entry::register_image(wxStyledTextCtrl* stc)
{
  stc->RegisterImage(
    std::to_underlying(image_access_t::PUBLIC),
    wxArtProvider::GetBitmap(wxART_PLUS));
  stc->RegisterImage(
    std::to_underlying(image_access_t::PROTECTED),
    wxArtProvider::GetBitmap(wxART_MINUS));
  stc->RegisterImage(
    std::to_underlying(image_access_t::PRIVATE),
    wxArtProvider::GetBitmap(wxART_TICK_MARK));
}

wex::ctags_entry& wex::ctags_entry::signature(const std::string& v)
{
  m_signature = v;
  return *this;
}

// signature:() const
// -> ()
// signature:(const std::string & s1)
// -> (
std::string skip_args_and_const(const std::string& text)
{
  if (text.empty())
  {
    return std::string();
  }

  if (text == "()")
  {
    return text;
  }

  if (wex::regex v("\\(\\).+"); v.match(text) == 0)
  {
    return "()";
  }

  if (wex::regex v("(\\(.+\\)).*"); v.match(text) == 1)
  {
    return "(";
  }

  return text;
}

std::string wex::ctags_entry::signature_and_image() const
{
  std::string s;

  if (is_function_or_prototype())
  {
    if (const auto* value = tagsField(&m_entry, "signature"); value != nullptr)
    {
      s.append(skip_args_and_const(value));
    }

    s.append(image_string());
  }

  return s;
}
