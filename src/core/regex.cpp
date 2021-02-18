////////////////////////////////////////////////////////////////////////////////
// Name:      core/regex.cpp
// Purpose:   Implementation of wex regex methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <wex/log.h>
#include <wex/regex.h>

int wex::match(
  const std::string&        reg,
  const std::string&        text,
  std::vector<std::string>& v)
{
  if (reg.empty())
    return -1;

  try
  {
    if (std::match_results<std::string::const_iterator> m;
        !std::regex_search(text, m, std::regex(reg)))
    {
      return -1;
    }
    else if (m.size() > 1)
    {
      v.clear();
      std::copy(++m.begin(), m.end(), std::back_inserter(v));
    }

    return v.size();
  }
  catch (std::regex_error& e)
  {
    log(e) << reg << "code:" << (int)e.code();
    return -1;
  }
}
