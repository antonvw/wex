////////////////////////////////////////////////////////////////////////////////
// Name:      reflection.cpp
// Purpose:   Declaration of wex::reflection class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/reflection.h>

const std::string to_string(const std::any& any)
{
  std::stringstream s;

  if (any.has_value())
  {
    try
    {
      if (any.type() == typeid(double))
      {
        s << std::any_cast<double>(any);
      }
      else if (any.type() == typeid(int))
      {
        s << std::any_cast<int>(any);
      }
      else if (any.type() == typeid(long))
      {
        s << std::any_cast<long>(any);
      }
      else if (any.type() == typeid(size_t))
      {
        s << std::any_cast<size_t>(any);
      }
      else if (any.type() == typeid(std::string))
      {
        s << std::any_cast<std::string>(any);
      }
      else
      {
        s << "<no cast available>";
      }
    }
    catch (const std::exception& e)
    {
      s << "<exception: " << e.what() << ">";
    }
  }
  else
  {
    s << "<no value>";
  }

  return s.str();
}

wex::reflection::reflection(const std::vector<reflection_t>& t, log_t what)
  : m_items(t)
  , m_log_t(what)
{
}

std::stringstream wex::reflection::log() const
{
  std::stringstream ss;
  bool              first = true;

  for (const auto& i : m_items)
  {
    const auto val(to_string(i.f())); // do not take reference

    if (m_log_t == log_t::SKIP_EMPTY && val.empty())
    {
      continue;
    }

    ss << (!first ? ", " : "") << i.name;

    if (!val.empty())
    {
      ss << ": " << val;
    }

    first = false;
  }

  if (!ss.str().empty())
  {
    ss << std::endl;
  }

  return ss;
}
